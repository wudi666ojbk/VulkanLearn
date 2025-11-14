#pragma once
#include "Renderer/Vulkan.h"

struct Buffer
{
	virtual VkBuffer GetVulkanBuffer() const = 0;

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
