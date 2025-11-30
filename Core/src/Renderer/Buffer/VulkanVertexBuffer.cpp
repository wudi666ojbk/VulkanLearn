#include "pch.h"
#include "VulkanVertexBuffer.h"

#include "Renderer/VulkanContext.h"

VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint64_t size)
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	Allocate(data, size, stagingBufferMemory);
	CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);
	CopyBuffer(stagingBuffer, m_VertexBuffer, size);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	Shutdown();
}

void VulkanVertexBuffer::Shutdown()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	// 确保设备空闲，避免缓冲区仍在使用时被销毁
	vkDeviceWaitIdle(device);

	vkDestroyBuffer(device, m_VertexBuffer, nullptr);
	vkFreeMemory(device, m_VertexBufferMemory, nullptr);
}

Ref<VulkanVertexBuffer> VulkanVertexBuffer::Create(void* data, uint64_t size)
{
	return CreateRef<VulkanVertexBuffer>(data, size);
}
