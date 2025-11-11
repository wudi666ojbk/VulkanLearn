#pragma once
#include "Vulkan.h"

#include "VulkanPipeline.h"
#include "Buffer/VulkanVertexBuffer.h"
#include "Buffer/VulkanIndexBuffer.h"

class VulkanRenderer
{
public:
	void Init(Ref<VulkanPipeline> pipeline);
	void Shutdown();

	static void DrawFrame();

	static void BeginRenderPass(Ref<VulkanPipeline> pipeline);
	static void EndRenderPass(VkCommandBuffer commandBuffer);
private:
	Ref<VulkanPipeline> m_Pipeline;
	Ref<VulkanVertexBuffer> m_VertexBuffer;
	Ref<VulkanIndexBuffer> m_IndexBuffer;
};

static VulkanRenderer* s_Renderer = nullptr;
