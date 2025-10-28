#pragma once
#include "Vulkan.h"
#include "VulkanPipeline.h"

class VulkanRenderer
{
public:
	static void BeginRenderPass(Ref<VulkanPipeline> pipeline);
	static void EndRenderPass(VkCommandBuffer commandBuffer);
private:

};

