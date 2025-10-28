#include "pch.h"
#include "VulkanRenderer.h"

#include "VulkanContext.h"

void VulkanRenderer::BeginRenderPass(Ref<VulkanPipeline> pipeline)
{
	VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
	VkCommandBuffer commandBuffer = swapChain.GetCurrentDrawCommandBuffer();
	VkRenderPass renderPass = swapChain.GetRenderPass();
	VkFramebuffer framebuffer = swapChain.GetCurrentFramebuffer();
	auto width = swapChain.GetSwapChainExtent().width;
	auto height = swapChain.GetSwapChainExtent().height;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChain.GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain.GetSwapChainExtent();

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	// 更新动态剪裁状态
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChain.GetSwapChainExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// 绑定 Vulkan Pipeline
	VkPipeline vulkanPipeline = pipeline->GetVulkanPipeline();
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	EndRenderPass(commandBuffer);
}

void VulkanRenderer::EndRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
}
