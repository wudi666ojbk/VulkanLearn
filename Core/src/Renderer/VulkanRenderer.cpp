#include "pch.h"
#include "VulkanRenderer.h"

#include "Application.h"
#include "VulkanContext.h"

void VulkanRenderer::Init(Ref<VulkanPipeline> pipeline)
{
	s_Renderer = this;
	m_Pipeline = pipeline;

	s_Renderer->m_Buffer = VulkanVertexBuffer::Create();
}

void VulkanRenderer::Shutdown()
{
	m_Buffer->Shutdown();
}

void VulkanRenderer::DrawFrame()
{
	auto& swapChain = Application::Get().GetWindow().GetSwapChain();

	swapChain.BeginFrame();
	
	// 获取当前帧的命令缓冲区
	VkCommandBuffer commandBuffer = swapChain.GetCurrentDrawCommandBuffer();
	
	// 开始记录命令缓冲区
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cmdBufInfo.pNext = nullptr;
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
	
	// 开始渲染过程
	BeginRenderPass(s_Renderer->m_Pipeline);

	VkBuffer vertexBuffers[] = { s_Renderer->m_Buffer->GetVulkanBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
	
	// 结束渲染过程
	EndRenderPass(commandBuffer);
	
	// 提交并呈现
	swapChain.Present();
}

void VulkanRenderer::BeginRenderPass(Ref<VulkanPipeline> pipeline)
{
	auto& swapChain = Application::Get().GetWindow().GetSwapChain();
	VkCommandBuffer commandBuffer = swapChain.GetCurrentDrawCommandBuffer();

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = swapChain.GetRenderPass();
	renderPassInfo.framebuffer = swapChain.GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain.GetSwapChainExtent();

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// 设置动态视口和剪裁区域
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChain.GetSwapChainExtent().width;
	viewport.height = (float)swapChain.GetSwapChainExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChain.GetSwapChainExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// 绑定 Vulkan Pipeline
	VkPipeline vulkanPipeline = pipeline->GetVulkanPipeline();
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);
}

void VulkanRenderer::EndRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
	// 结束命令缓冲区记录
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
}