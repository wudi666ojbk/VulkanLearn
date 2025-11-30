#include "pch.h"
#include "VulkanIndexBuffer.h"

#include "Renderer/VulkanContext.h"

VulkanIndexBuffer::VulkanIndexBuffer(void* data, uint64_t size)
    : m_Size(size)
{
    m_LocalData = Buffer::Copy(data, size);

    auto device = VulkanContext::Get()->GetCurrentDevice();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    Allocate(data, size, stagingBufferMemory);

    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

    CopyBuffer(stagingBuffer, m_IndexBuffer, size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();

    vkDestroyBuffer(device, m_IndexBuffer, nullptr);
    vkFreeMemory(device, m_IndexBufferMemory, nullptr);
}

Ref<VulkanIndexBuffer> VulkanIndexBuffer::Create(void* data, uint64_t size)
{
    return CreateRef<VulkanIndexBuffer>(data, size);
}
