#pragma once
#include "Renderer/Vulkan.h"

struct Buffer
{
	void* Data = nullptr;
	uint64_t Size = 0;

	Buffer() = default;

	Buffer(const void* data, uint64_t size = 0)
		: Data((void*)data), Size(size) {
	}
};

struct VulkanBuffer
{
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void Allocate(const void* dstdata, VkDeviceSize size, VkDeviceMemory memory);
};