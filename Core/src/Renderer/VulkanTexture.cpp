#include "pch.h"
#include "VulkanTexture.h"

#include "Buffer/Buffer.h"
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
    auto device = VulkanContext::Get()->GetCurrentDevice();

    Utils::ValidateSpecification(specification);

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

    ToBufferFromFile();
}

VulkanTexture::~VulkanTexture()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();

    vkDestroyImage(device, m_Image, nullptr);
    vkFreeMemory(device, m_DeviceMemory, nullptr);
}

Ref<VulkanTexture> VulkanTexture::Create(const TextureSpecification& specification, const std::filesystem::path& filepath)
{
	return CreateRef<VulkanTexture>(specification, filepath);
}

void VulkanTexture::CopyBufferToImage(VkBuffer buffer)
{
    auto device = VulkanContext::Get()->GetDevice();

    
}

void VulkanTexture::ToBufferFromFile()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();
    auto vkDevice = VulkanContext::Get()->GetDevice();
    auto physicalDevice = VulkanContext::Get()->GetPhysicalDevice();

    int width, height, channels;

    // 读取图片
    // TODO: 这里应该根据具体的格式选择合适的格式
    stbi_uc* pixels = stbi_load(m_Path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    m_Size = width * height * 4;

    CORE_ASSERT(pixels);

    m_Specification.Width = width;
    m_Specification.Height = height;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    Buffer buffer;
    buffer.CreateBuffer(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);
    buffer.Allocate(pixels, m_Size, stagingMemory);

    // 释放以加载的图片
    stbi_image_free(pixels);
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

    VkCommandBuffer commandBuffer = vkDevice->GetCommandBuffer(true);

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