#pragma once
#include "Vulkan.h"
#include <GLFW/glfw3.h>

#include "VulkanDevice.h"

struct GLFWwindow;

class VulkanSwapChain
{
public:
	void Create(uint32_t* width, uint32_t* height);
	void Init(VkInstance instance, const Ref<VulkanDevice>& device);
	void InitSurface(GLFWwindow* windowHandle);

	void Destroy();

	// 成员获取
	VkRenderPass GetRenderPass() { return m_RenderPass; }
private:
	void FindImageFormatAndColorSpace();	// 找到适合的颜色格式和色彩空间
	void CreateImageViews();				// 创建图像视图
	void CreateRenderPass();
private:
	// Vulkan实例
	VkInstance m_Instance = nullptr;

	// Vulkan渲染通道
	VkRenderPass m_RenderPass = nullptr;    // 用于屏幕渲染的RenderPass

	// Vulkan设备
	Ref<VulkanDevice> m_Device;

	// 是否开启垂直同步
	bool m_VSync = false;

	// Vulkan交换链
	VkSwapchainKHR m_SwapChain;
	uint32_t m_ImageCount = 0;
	std::vector<VkImage> m_VulkanImages;

	// 交换链图像结构体
	struct SwapchainImage
	{
		VkImage Image = nullptr;
		VkImageView ImageView = nullptr;
	};
	std::vector<SwapchainImage> m_Images;

	// 交换链颜色格式和颜色空间
	VkFormat m_ColorFormat;
	VkColorSpaceKHR m_ColorSpace;

	// 交换链图像范围
	VkExtent2D m_SwapChainExtent;

	// Vulkan表面
	VkSurfaceKHR m_Surface;

	// 允许VulkanContext访问私有成员
	friend class VulkanContext;
};

