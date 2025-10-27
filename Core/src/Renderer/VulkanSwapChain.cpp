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
	VkSwapchainCreateInfoKHR swapchainCI = {};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.surface = m_Surface;
	swapchainCI.minImageCount = imageCount;
	swapchainCI.imageFormat = m_ColorFormat;
	swapchainCI.imageColorSpace = m_ColorSpace;;
	swapchainCI.imageExtent = { m_SwapChainExtent.width, m_SwapChainExtent.height };
	// 指定交换链图像的数组层数（除非开发立体3D应用，否则总是1）
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.pQueueFamilyIndices = NULL;
	// 指定交换链图像的用途（直接渲染到这些图像上）
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// 指定变换（这里使用默认变换）
	swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	// 指定alpha通道（这里设置为不透明）
	swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// 使用之前选择的呈现模式
	swapchainCI.presentMode = swapchainPresentMode;
	// 允许裁剪（隐藏的像素不需要更新）
	swapchainCI.clipped = VK_TRUE;
	swapchainCI.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &m_SwapChain);

	CreateImageViews();
	CreateRenderPass();
	CreateFramebuffers();
	CreateCommandBuffers();
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

	GetQueueNodeIndex();
	FindImageFormatAndColorSpace();
}

void VulkanSwapChain::Destroy()
{
	auto device = m_Device->GetVulkanDevice();
	vkDeviceWaitIdle(device);

	for (auto &imageView : m_Images)
		vkDestroyImageView(m_Device->GetVulkanDevice(), imageView.ImageView, nullptr);
	m_Images.clear();
	for (auto& commandBuffer : m_CommandBuffers)
		vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);
	m_CommandBuffers.clear();
	for (auto framebuffer : m_Framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	m_Framebuffers.clear();

	vkDestroyRenderPass(device, m_RenderPass, nullptr);
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

void VulkanSwapChain::GetQueueNodeIndex()
{
	auto physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

	// 获取可用队列族属性
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
	CORE_ASSERT(queueCount >= 1);

	std::vector<VkQueueFamilyProperties> queueProps(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

	// 遍历每个队列以了解它是否支持呈现：
	// 找到一个支持呈现的队列
	// 将用于将交换链图像呈现给窗口系统
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &supportsPresent[i]);
	}

	// 在队列族数组中搜索图形队列和呈现队列
	// 尝试找到一个同时支持两者队列
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// 如果没有一个队列同时支持呈现和图形
		// 尝试找到一个单独的呈现队列
		for (uint32_t i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	CORE_ASSERT(graphicsQueueNodeIndex != UINT32_MAX);
	CORE_ASSERT(presentQueueNodeIndex != UINT32_MAX);

	m_QueueNodeIndex = graphicsQueueNodeIndex;
}

void VulkanSwapChain::CreateImageViews()
{
	VkDevice device = m_Device->GetVulkanDevice();

	// 获取交换链图像
	vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, nullptr);
	m_VulkanImages.resize(m_ImageCount);
	vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, m_VulkanImages.data());

	// 获取包含图像和图像视图的交换链缓冲区
	m_Images.resize(m_ImageCount);

	for (uint32_t i = 0; i < m_ImageCount; i++)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.image = m_VulkanImages[i];

		// 指定图像类型（2D纹理）
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		// 使用交换链图像格式
		colorAttachmentView.format = m_ColorFormat;

		// 指定颜色通道映射（默认的RGBA映射）
		colorAttachmentView.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		colorAttachmentView.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		colorAttachmentView.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		colorAttachmentView.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// 指定图像子资源范围
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;

		m_Images[i].Image = m_VulkanImages[i];
		vkCreateImageView(device, &colorAttachmentView, nullptr, &m_Images[i].ImageView);
	}
}

void VulkanSwapChain::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_ColorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// 子passes
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VK_CHECK_RESULT(vkCreateRenderPass(m_Device->GetVulkanDevice(), &renderPassInfo, nullptr, &m_RenderPass));
}

void VulkanSwapChain::CreateFramebuffers()
{
	m_Framebuffers.resize(m_ImageCount);

	for (size_t i = 0; i < m_ImageCount; i++) {
		VkImageView attachments[] = {
			m_Images[i].ImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_SwapChainExtent.width;
		framebufferInfo.height = m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_Device->GetVulkanDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
	}
}

void VulkanSwapChain::CreateCommandBuffers()
{
	auto device = m_Device->GetVulkanDevice();

	for (auto& commandBuffer : m_CommandBuffers)
		vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	cmdPoolInfo.queueFamilyIndex = m_QueueNodeIndex;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	m_CommandBuffers.resize(m_ImageCount);
	for (auto& commandBuffer : m_CommandBuffers)
	{
		VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandBuffer.CommandPool));

		commandBufferAllocateInfo.commandPool = commandBuffer.CommandPool;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer.CommandBuffer));
	}

	// Record command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	for (auto& commandBuffer : m_CommandBuffers)
		vkBeginCommandBuffer(commandBuffer.CommandBuffer, &beginInfo);
}
