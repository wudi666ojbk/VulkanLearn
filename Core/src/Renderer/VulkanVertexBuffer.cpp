#include "pch.h"
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"

VulkanVertexBuffer::VulkanVertexBuffer()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();
	auto physicalDevice = VulkanContext::Get()->GetPhysicalDevice()->GetVulkanPhysicalDevice();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();

	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_VertexBuffer));

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_MemoryProperties);
	vkGetBufferMemoryRequirements(device, m_VertexBuffer, &m_MemRequirements);

	Allocate();

	vkBindBufferMemory(device, m_VertexBuffer, m_VertexBufferMemory, 0);

	void* data;
	vkMapMemory(device, m_VertexBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(device, m_VertexBufferMemory);

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

void VulkanVertexBuffer::Allocate()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = m_MemRequirements.size;
	allocInfo.memoryTypeIndex = GetMemoryTypeIndex(m_MemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &m_VertexBufferMemory));
}

uint32_t VulkanVertexBuffer::GetMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++) 
	{
		if ((typeFilter & (1 << i)) && (m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
