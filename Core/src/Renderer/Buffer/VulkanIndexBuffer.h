#pragma once
#include "Renderer/Vulkan.h"

#include "Buffer.h"

class VulkanIndexBuffer : public VulkanBuffer
{
public:
	VulkanIndexBuffer();
	~VulkanIndexBuffer();

	static Ref<VulkanIndexBuffer> Create();

	VkBuffer GetVulkanBuffer() const { return m_IndexBuffer; }
private:
	VkBuffer m_IndexBuffer = nullptr;
	VkDeviceMemory m_IndexBufferMemory = nullptr;

};


