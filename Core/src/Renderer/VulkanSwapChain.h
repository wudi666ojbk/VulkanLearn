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
private:
	void FindImageFormatAndColorSpace();   // 找到适合的颜色格式和色彩空间
private:
	VkInstance m_Instance = nullptr;
	Ref<VulkanDevice> m_Device;
	bool m_VSync = false;				// 是否开启垂直同步

	VkSwapchainKHR m_SwapChain;
	uint32_t m_ImageCount = 0;
	std::vector<VkImage> m_VulkanImages;

	VkFormat m_ColorFormat;
	VkColorSpaceKHR m_ColorSpace;

	VkExtent2D m_SwapChainExtent;

	VkRenderPass m_RenderPass = nullptr;

	VkSurfaceKHR m_Surface;

	friend class VulkanContext;
};

