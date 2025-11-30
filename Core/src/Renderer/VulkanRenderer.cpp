#include "pch.h"
#include "VulkanRenderer.h"

#include "Application.h"
#include "VulkanContext.h"
#include "Data/Vertex.h"

#include "VulkanTexture.h"
#include <tiny_obj_loader.h>

struct VulkanRendererData
{
	Ref<VulkanVertexBuffer> VertexBuffer;
	Ref<VulkanIndexBuffer> IndexBuffer;
	Ref<VulkanUniformBuffer> UniformBuffer;

	VulkanShader::ShaderDescriptorSet shaderDescriptorSet;
};

// 临时数据
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

static VulkanRendererData* s_Data = nullptr;

void VulkanRenderer::Init(Ref<VulkanPipeline> pipeline)
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	s_Renderer = this;
	m_Pipeline = pipeline;

	s_Data = new VulkanRendererData();

	// 加载模型数据
	const std::string MODEL_PATH = "models/viking_room.obj";
	const std::string TEXTURE_PATH = "textures/viking_room.png";

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	// 创建缓冲区
	s_Data->VertexBuffer = VulkanVertexBuffer::Create((void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	s_Data->IndexBuffer = VulkanIndexBuffer::Create((void*)indices.data(), indices.size() * sizeof(indices[0]));
	s_Data->UniformBuffer = VulkanUniformBuffer::Create(); 

	TextureSpecification textureSpec;
	m_Texture = VulkanTexture::Create(textureSpec, TEXTURE_PATH);

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
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

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