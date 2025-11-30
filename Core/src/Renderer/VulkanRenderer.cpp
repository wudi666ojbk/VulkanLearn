#include "pch.h"
#include "VulkanRenderer.h"

#include "Application.h"
#include "VulkanContext.h"
#include "Data/Vertex.h"

#include "VulkanTexture.h"

struct VulkanRendererData
{
	Ref<VulkanVertexBuffer> VertexBuffer;
	Ref<VulkanIndexBuffer> IndexBuffer;
	Ref<VulkanUniformBuffer> UniformBuffer;

	VulkanShader::ShaderDescriptorSet shaderDescriptorSet;
};

static VulkanRendererData* s_Data = nullptr;

void VulkanRenderer::Init(Ref<VulkanPipeline> pipeline)
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	s_Renderer = this;
	m_Pipeline = pipeline;

	s_Data = new VulkanRendererData();

	s_Data->VertexBuffer = VulkanVertexBuffer::Create((void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	s_Data->IndexBuffer = VulkanIndexBuffer::Create((void*)indices.data(), indices.size() * sizeof(indices[0]));
	s_Data->UniformBuffer = VulkanUniformBuffer::Create(); 

	std::filesystem::path filePath = "textures/texture.jpg";
	TextureSpecification textureSpec;
	textureSpec.Width = 1280;
	textureSpec.Height = 960;
	m_Texture = VulkanTexture::Create(textureSpec, filePath);

	uint32_t framesInFlight = VulkanContext::Get()->GetConfig().FramesInFlight; // 获取最大飞行帧数

	auto shader = pipeline->GetShader();

	s_Data->shaderDescriptorSet = shader->CreateDescriptorSets(); // 创建描述符集

	VkDescriptorSetLayout descriptorSetLayout = shader->GetDescriptorSetLayout(); // 获取描述符布局

	std::vector<VkBuffer> uniformBuffer = s_Data->UniformBuffer->GetVulkanBuffer();
	for (size_t i = 0; i < framesInFlight; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffer[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_Texture->GetImageView();
		imageInfo.sampler = m_Texture->GetSampler();
		
		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = s_Data->shaderDescriptorSet.DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = s_Data->shaderDescriptorSet.DescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanRenderer::Shutdown()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	vkDestroyDescriptorPool(device, s_Data->shaderDescriptorSet.Pool, nullptr);
	
	m_Texture.reset(); // 显式释放纹理资源

	delete s_Data;
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

	VkBuffer vertexBuffers[] = { s_Data->VertexBuffer->GetVulkanBuffer() };
	VkBuffer indexBuffer = s_Data->IndexBuffer->GetVulkanBuffer();
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	// 绑定描述符集
	auto pipelineLayout = s_Renderer->m_Pipeline->GetVulkanPipelineLayout();
	std::vector<VkDescriptorSet> descriptorSets = s_Data->shaderDescriptorSet.DescriptorSets;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[swapChain.GetCurrentImageIndex()], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	
	// 结束渲染过程
	EndRenderPass(commandBuffer);
	
	// 提交并呈现
	swapChain.Present();
}

void VulkanRenderer::BeginRenderPass(Ref<VulkanPipeline> pipeline)
{
	auto& swapChain = Application::Get().GetWindow().GetSwapChain();
	VkCommandBuffer commandBuffer = swapChain.GetCurrentDrawCommandBuffer();

	s_Data->UniformBuffer->UpdateUniformBuffer(swapChain.GetCurrentImageIndex());

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = swapChain.GetRenderPass();
	renderPassInfo.framebuffer = swapChain.GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain.GetSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

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

namespace Utils{

	void InsertImageMemoryBarrier(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}
}