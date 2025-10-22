#pragma once
#include "Vulkan.h"
#include <GLFW/glfw3.h>

#include "VulkanDevice.h"

struct GLFWwindow;

class VulkanSwapChain
{
public:
	void Init(VkInstance instance, const Ref<VulkanDevice>& device);
	void InitSurface(GLFWwindow* windowHandle);

	void Destroy();

	// 成员获取

private:
	VkInstance m_Instance = nullptr;
	Ref<VulkanDevice> m_Device;
	bool m_VSync = false;				// 是否开启垂直同步

	VkSwapchainKHR m_SwapChain = nullptr;

	VkRenderPass m_RenderPass = nullptr;

	VkSurfaceKHR m_Surface;

	friend class VulkanContext;
};

