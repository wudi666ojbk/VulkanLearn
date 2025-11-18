#pragma once
#include "Vulkan.h"

class VulkanCommandPool
{
public:
	VulkanCommandPool();
	~VulkanCommandPool();

	VkCommandBuffer AllocateCommandBuffer(bool begin, bool compute);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

	VkCommandPool GetGraphicsCommandPool() const { return m_GraphicsCommandPool; }
	VkCommandPool GetComputeCommandPool() const { return m_ComputeCommandPool; }
private:
	VkCommandPool m_GraphicsCommandPool, m_ComputeCommandPool;
};

