#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Renderer/Vulkan.h>
#include <Renderer/VulkanDevice.h>

class VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

	static Ref<VulkanContext> Create();
	void Init();

	static Ref<VulkanContext> Get();

	static VkInstance GetInstance() { return s_VulkanInstance; }
	Ref<VulkanDevice> GetDevice() const { return m_Device; }
	Ref<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_PhysicalDevice; }
private:
	bool CheckValidationLayerSupport();

	Ref<VulkanDevice> m_Device;
	Ref<VulkanPhysicalDevice> m_PhysicalDevice;

	VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;

	inline static VkInstance s_VulkanInstance;
};