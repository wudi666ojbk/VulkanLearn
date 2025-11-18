#include "pch.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"

VulkanCommandPool::VulkanCommandPool()
{
	auto device = VulkanContext::Get()->GetDevice();
	auto vulkanDevice = device->GetVulkanDevice();

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(vulkanDevice, &cmdPoolInfo, nullptr, &m_GraphicsCommandPool));

	cmdPoolInfo.queueFamilyIndex = device->GetPhysicalDevice()->GetQueueFamilyIndices().Compute;
	VK_CHECK_RESULT(vkCreateCommandPool(vulkanDevice, &cmdPoolInfo, nullptr, &m_ComputeCommandPool));
}

VulkanCommandPool::~VulkanCommandPool()
{
	auto device = VulkanContext::Get()->GetDevice();
	auto vulkanDevice = device->GetVulkanDevice();

	vkDestroyCommandPool(vulkanDevice, m_GraphicsCommandPool, nullptr);
	vkDestroyCommandPool(vulkanDevice, m_ComputeCommandPool, nullptr);
}

VkCommandBuffer VulkanCommandPool::AllocateCommandBuffer(bool begin, bool compute)
{
	auto device = VulkanContext::Get()->GetDevice();
	auto vulkanDevice = device->GetVulkanDevice();

	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = compute ? m_ComputeCommandPool : m_GraphicsCommandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkanDevice, &cmdBufAllocateInfo, &cmdBuffer));

	if (begin)
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
	}

	return cmdBuffer;
}

void VulkanCommandPool::FlushCommandBuffer(VkCommandBuffer commandBuffer)
{
	auto device = VulkanContext::Get()->GetDevice();

	FlushCommandBuffer(commandBuffer, device->GetGraphicsQueue());
}

void VulkanCommandPool::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
{
	auto device = VulkanContext::Get()->GetDevice();
	CORE_ASSERT(queue == device->GetGraphicsQueue());
	auto vulkanDevice = device->GetVulkanDevice();

	const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

	CORE_ASSERT(commandBuffer != VK_NULL_HANDLE);

	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	VK_CHECK_RESULT(vkCreateFence(vulkanDevice, &fenceCreateInfo, nullptr, &fence));

	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK_RESULT(vkWaitForFences(vulkanDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(vulkanDevice, fence, nullptr);
	vkFreeCommandBuffers(vulkanDevice, m_GraphicsCommandPool, 1, &commandBuffer);
}
