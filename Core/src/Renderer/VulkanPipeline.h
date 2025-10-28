#pragma once
#include "Vulkan.h"
#include "VulkanShader.h"
#include "VulkanSwapChain.h"

class VulkanPipeline
{
public:
	VulkanPipeline(Ref<VulkanShader> shader, VulkanSwapChain* swapChain);
	~VulkanPipeline();

	static Ref<VulkanPipeline> Create(Ref<VulkanShader> shader, VulkanSwapChain* swapChain);

	void Invalidate();

	// 获取成员变量
	VkPipelineLayout GetVulkanPipelineLayout() const { return m_PipelineLayout; }
	VkPipeline GetVulkanPipeline() { return m_Pipeline; }
private:
private:
	Ref<VulkanShader> m_Shader;
	VulkanSwapChain* m_SwapChain;

	VkPipeline m_Pipeline = nullptr;
	VkPipelineLayout m_PipelineLayout = nullptr;
};

