#pragma once
#include "Vulkan.h"
#include "VulkanShader.h"
#include "VulkanSwapChain.h"

class VulkanPipeline
{
public:
	VulkanPipeline(Ref<VulkanShader> shader, VulkanSwapChain* swapChain);
	~VulkanPipeline();

	void Invalidate();

	static Ref<VulkanPipeline> Create(Ref<VulkanShader> shader, VulkanSwapChain* swapChain);

	// 获取成员变量
	VkPipelineLayout GetVulkanPipelineLayout() const { return m_PipelineLayout; }
private:
private:
	Ref<VulkanShader> m_Shader;
	VulkanSwapChain* m_SwapChain;

	VkPipeline m_Pipeline = nullptr;
	VkPipelineLayout m_PipelineLayout = nullptr;
};

