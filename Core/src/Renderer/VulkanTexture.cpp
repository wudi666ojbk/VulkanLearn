#include "pch.h"
#include "VulkanTexture.h"

#include "VulkanContext.h"
#include "VulkanRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Utils {
    static bool ValidateSpecification(const TextureSpecification& specification)
    {
        bool result = true;

        result = specification.Width > 0 && specification.Height > 0 && specification.Width < 65536 && specification.Height < 65536;
        CORE_ASSERT(result);

        return result;
    }
}

VulkanTexture::VulkanTexture(const TextureSpecification& specification, const std::filesystem::path& filepath)
    : m_Specification(specification), m_Path(filepath)
{
    auto vkDevice = VulkanContext::Get()->GetDevice();
    auto device = vkDevice->GetVulkanDevice();
    VkCommandBuffer commandBuffer = vkDevice->GetCommandBuffer(true);
    auto physicalDevice = VulkanContext::Get()->GetPhysicalDevice();

    Utils::ValidateSpecification(specification);

    m_ImageData = ToBufferFromFile(filepath, m_Specification.Width, m_Specification.Height);

    // 在设备上创建最优分块目标图像
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = { m_Specification.Width, m_Specification.Height, 1 };
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    // TODO: 这里应该根据具体的格式选择合适的格式
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &m_Image));

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VulkanBuffer buffer;
    buffer.CreateBuffer(m_ImageData.Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);
    buffer.Allocate(m_ImageData.Data, m_ImageData.Size, stagingMemory);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_Image, &memRequirements);

    physicalDevice->GetMemoryProperties();

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = physicalDevice->GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &m_DeviceMemory));

    vkBindImageMemory(device, m_Image, m_DeviceMemory, 0);

    TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        m_Specification.Width,
        m_Specification.Height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkDevice->FlushCommandBuffer(commandBuffer);

    TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    CreateTextureImageView();
    CreateTextureSampler();
}

VulkanTexture::~VulkanTexture()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();

    vkDestroyImage(device, m_Image, nullptr);
    vkFreeMemory(device, m_DeviceMemory, nullptr);

    vkDestroySampler(device, m_Sampler, nullptr);
    vkDestroyImageView(device, m_ImageView, nullptr);
}

Ref<VulkanTexture> VulkanTexture::Create(const TextureSpecification& specification, const std::filesystem::path& filepath)
{
	return CreateRef<VulkanTexture>(specification, filepath);
}

void VulkanTexture::CreateTextureImageView()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView));
}

void VulkanTexture::CreateTextureSampler()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler));
}

void VulkanTexture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) 
{
    auto device = VulkanContext::Get()->GetDevice();

    VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    device->FlushCommandBuffer(commandBuffer);
}

Buffer VulkanTexture::ToBufferFromFile(const std::filesystem::path& path, uint32_t& outWidth, uint32_t& outHeight)
{
    std::string pathString = path.string();

    int width, height, channels;
    void* tmp;
    size_t size = 0;

    if (stbi_is_hdr(pathString.c_str()))
    {
        tmp = stbi_loadf(pathString.c_str(), &width, &height, &channels, 4);
        if (tmp)
        {
            size = width * height * 4 * sizeof(float);
        }
    }
    else
    {
        //stbi_set_flip_vertically_on_load(1);
        tmp = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
        if (tmp)
        {
            size = width * height * 4;
        }
    }

    if (!tmp)
    {
        return {};
    }

    Buffer imageBuffer;

    CORE_ASSERT(size > 0);
    imageBuffer.Data = new uint8_t[size]; // avoid `malloc+delete[]` mismatch.
    imageBuffer.Size = size;
    memcpy(imageBuffer.Data, tmp, size);
    stbi_image_free(tmp);

    outWidth = width;
    outHeight = height;
    return imageBuffer;
}
