#pragma once
#include "Renderer/Vulkan.h"

#include "Buffer.h"

class VulkanIndexBuffer : public VulkanBuffer
{
public:
	VulkanIndexBuffer(void* data, uint64_t size);
	~VulkanIndexBuffer();

	static Ref<VulkanIndexBuffer> Create(void* data, uint64_t size = 0);

	VkBuffer GetVulkanBuffer() const { return m_IndexBuffer; }

	uint64_t GetSize() const { return m_Size; }
private:
	uint64_t m_Size = 0;

	Buffer m_LocalData;

	VkBuffer m_IndexBuffer = nullptr;
	VkDeviceMemory m_IndexBufferMemory = nullptr;

};


