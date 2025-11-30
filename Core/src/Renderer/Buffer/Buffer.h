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

	static Buffer Copy(const Buffer& other)
	{
		Buffer buffer;
		buffer.Allocate(other.Size);
		memcpy(buffer.Data, other.Data, other.Size);
		return buffer;
	}

	static Buffer Copy(const void* data, uint64_t size)
	{
		Buffer buffer;
		buffer.Allocate(size);
		if (size) memcpy(buffer.Data, data, size);
		return buffer;
	}

	void Allocate(uint64_t size)
	{
		delete[](uint8_t*)Data;
		Data = nullptr;
		Size = size;

		if (size == 0)
			return;

		Data = new uint8_t[size];
	}

	void Release()
	{
		delete[](uint8_t*)Data;
		Data = nullptr;
		Size = 0;
	}
};

struct VulkanBuffer
{
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void Allocate(const void* dstdata, VkDeviceSize size, VkDeviceMemory memory);
};