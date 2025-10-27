#pragma once
#include "Vulkan.h"

class VulkanPipline
{
public:
	VulkanPipline();
	~VulkanPipline();

	static Ref<VulkanPipline> Create();

	// 获取成员变量
	VkPipelineLayout GetVulkanPipelineLayout() { return m_PipelineLayout; }
private:
private:
	VkPipelineLayout m_PipelineLayout = nullptr;
};

