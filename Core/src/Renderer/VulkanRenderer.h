#pragma once
#include "Vulkan.h"

#include "VulkanPipeline.h"
#include "Buffer/VulkanVertexBuffer.h"
#include "Buffer/VulkanIndexBuffer.h"
#include "Buffer/VulkanUniformBuffer.h"

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
};

static VulkanRenderer* s_Renderer = nullptr;
