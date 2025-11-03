#pragma once
#include "Vulkan.h"
#include "VulkanPipeline.h"
#include "VulkanVertexBuffer.h"

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
	Ref<VulkanVertexBuffer> m_Buffer;
};

static VulkanRenderer* s_Renderer = nullptr;
