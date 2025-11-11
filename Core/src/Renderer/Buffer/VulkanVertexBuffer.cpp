#include "pch.h"
#include "VulkanVertexBuffer.h"

#include "Renderer/VulkanContext.h"
#include "Data/Vertex.h"


VulkanVertexBuffer::VulkanVertexBuffer()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

	CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

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

Ref<VulkanVertexBuffer> VulkanVertexBuffer::Create()
{
	return CreateRef<VulkanVertexBuffer>();
}
