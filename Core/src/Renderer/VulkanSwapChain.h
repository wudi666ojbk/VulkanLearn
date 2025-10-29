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

	void DrawFrame();
	void Present();

	void OnResize(uint32_t width, uint32_t height);

	// 成员获取
	VkRenderPass GetRenderPass() { return m_RenderPass; }
	VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }

	// TODO: m_ImageCount临时用于获取当前帧缓冲区，后续需要修改
	VkFramebuffer GetCurrentFramebuffer() { return GetFramebuffer(m_CurrentImageIndex); }
	VkFramebuffer GetFramebuffer(uint32_t index) { return m_Framebuffers[index]; }

	VkCommandBuffer GetCurrentDrawCommandBuffer() { return GetDrawCommandBuffer(m_CurrentFrameIndex); }
	VkCommandBuffer GetDrawCommandBuffer(uint32_t index) { return m_CommandBuffers[index].CommandBuffer; }
private:
	uint32_t AcquireNextImage();			// 获取下一个可用的图像索引
	void FindImageFormatAndColorSpace();	// 找到适合的颜色格式和色彩空间
	// 获取支持的队列
	void GetQueueNodeIndex();

	void CreateSwapChain(uint32_t* width, uint32_t* height);
	void CreateImageViews();				// 创建图像视图
	void CreateRenderPass();				// 创建渲染Pass
	void CreateFramebuffers();				// 创建帧缓冲区
	void CreateCommandBuffers();			// 创建命令缓冲区
	void CreateSyncObjects();				// 创建同步对象
private:
	// Vulkan实例
	VkInstance m_Instance = nullptr;

	// Vulkan渲染通道
	VkRenderPass m_RenderPass = nullptr;    // 用于屏幕渲染的RenderPass
	uint32_t m_CurrentFrameIndex = 0;		// 当前正在处理的帧的索引，最多飞行帧数
	uint32_t m_CurrentImageIndex = 0;		// 当前交换链图像的索引。 可能与帧索引不同

	// Vulkan设备
	Ref<VulkanDevice> m_Device;

	// 是否开启垂直同步
	bool m_VSync = false;

	// Vulkan交换链
	VkSwapchainKHR m_SwapChain;
	uint32_t m_ImageCount = 0;
	std::vector<VkImage> m_VulkanImages;

	// 交换链图像范围
	uint32_t m_Width = 0, m_Height = 0;

	// 交换链图像结构体
	struct SwapchainImage
	{
		VkImage Image = nullptr;
		VkImageView ImageView = nullptr;
	};
	std::vector<SwapchainImage> m_Images;

	// Semaphores to signal that images are available for rendering and that rendering has finished (one pair for each frame in flight)
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	// Fences to signal that command buffers are ready to be reused (one for each frame in flight)
	std::vector<VkFence> m_WaitFences;

	// 命令缓冲区
	struct SwapchainCommandBuffer
	{
		VkCommandPool CommandPool = nullptr;
		VkCommandBuffer CommandBuffer = nullptr;
	};
	std::vector<SwapchainCommandBuffer> m_CommandBuffers;
	uint32_t m_QueueNodeIndex = UINT32_MAX;

	// 交换链颜色格式和颜色空间
	VkFormat m_ColorFormat;
	VkColorSpaceKHR m_ColorSpace;

	// 交换链图像范围
	VkExtent2D m_SwapChainExtent;

	// Vulkan表面
	VkSurfaceKHR m_Surface;

	// 帧缓冲区
	std::vector<VkFramebuffer> m_Framebuffers;

	// 允许VulkanContext访问私有成员
	friend class VulkanContext;
};

