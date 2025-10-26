#include "pch.h"
#include "VulkanSwapChain.h"

void VulkanSwapChain::Create(uint32_t* width, uint32_t* height)
{
	VkDevice device = m_Device->GetVulkanDevice();
	VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

	// 创建交换链需要以下类型的设置
	// 1.表面格式（颜色深度）
	// 2.呈现模式（将图像“交换”到屏幕的条件）
	// 3.交换范围（交换链中图像的分辨率）

	// 获取支持的表面格式列表
	VkSurfaceCapabilitiesKHR surfCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps);

	// 设置交换链分辨率
	if (surfCaps.currentExtent.width == (uint32_t)-1)
	{
		// 如果表面大小未定义，则大小设置为请求的图像大小
		m_SwapChainExtent.width = *width;
		m_SwapChainExtent.height = *height;
	}
	else
	{
		// 如果表面大小已定义，则交换链大小必须匹配
		m_SwapChainExtent = surfCaps.currentExtent;
		*width = surfCaps.currentExtent.width;
		*height = surfCaps.currentExtent.height;
	}

	// 获取可用的呈现模式
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
	CORE_ASSERT(presentModeCount > 0);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data()));

	// 为交换链选择一个呈现模式
	// VK_PRESENT_MODE_FIFO_KHR：
	// 交换链是一个队列，显示器会在刷新时从队列的前面获取图像，程序会将渲染的图像插入队列的后面。 
	// 如果队列已满，则程序必须等待。 这与现代游戏中发现的垂直同步最相似。 显示器刷新的时刻称为“垂直消隐”。

	// VK_PRESENT_MODE_FIFO_RELAXED_KHR：
	// 如果应用程序迟到且队列在上次垂直消隐时为空，则此模式与前一种模式的区别仅在于此。
	// 当图像最终到达时，它会立即传输，而不是等待下一个垂直消隐。 这可能会导致明显的撕裂。

	// VK_PRESENT_MODE_MAILBOX_KHR：
	// 这是第二种模式的另一种变体。 当队列已满时，不是阻塞应用程序，而是简单地将已排队的图像替换为较新的图像。
	// 此模式可用于尽可能快地渲染帧，同时仍避免撕裂，从而导致比标准垂直同步更少的延迟问题。 
	// 这通常被称为“三重缓冲”，尽管仅存在三个缓冲区并不一定意味着帧率已解锁。
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}


	// 确定图像数量
	uint32_t imageCount = surfCaps.minImageCount + 1;
	if ((surfCaps.maxImageCount > 0) && (imageCount > surfCaps.maxImageCount))
	{
		imageCount = surfCaps.maxImageCount;
	}

	// 查找表面的变换
	VkSurfaceTransformFlagsKHR preTransform;
	preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	// 交换链创建信息
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = m_ColorFormat;
	createInfo.imageColorSpace = m_ColorSpace;;
	createInfo.imageExtent = { m_SwapChainExtent.width, m_SwapChainExtent.height };
	// 指定交换链图像的数组层数（除非开发立体3D应用，否则总是1）
	createInfo.imageArrayLayers = 1;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = NULL;
	// 指定交换链图像的用途（直接渲染到这些图像上）
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// 指定变换（这里使用默认变换）
	createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	// 指定alpha通道（这里设置为不透明）
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// 使用之前选择的呈现模式
	createInfo.presentMode = swapchainPresentMode;
	// 允许裁剪（隐藏的像素不需要更新）
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain);

	// 获取交换链图像
	vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, nullptr);
	m_VulkanImages.resize(m_ImageCount);
	vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, m_VulkanImages.data());
}

void VulkanSwapChain::Init(VkInstance instance, const Ref<VulkanDevice>& device)
{
	m_Instance = instance;
	m_Device = device;

	VkDevice vulkanDevice = m_Device->GetVulkanDevice();
}

void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
{
	VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

	glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface);

	FindImageFormatAndColorSpace();
}

void VulkanSwapChain::Destroy()
{
	auto device = m_Device->GetVulkanDevice();
	vkDeviceWaitIdle(device);

	vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

	vkDeviceWaitIdle(device);
}

void VulkanSwapChain::FindImageFormatAndColorSpace()
{
	VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

	// 获取支持的表面格式列表
	uint32_t formatCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr))
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.data()))

	// 如果表面格式列表仅包含一个 VK_FORMAT_UNDEFINED 条目，
	// 则没有首选格式，所以我们假设 VK_FORMAT_B8G8R8A8_UNORM
	m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	m_ColorSpace = surfaceFormats[0].colorSpace;
}
