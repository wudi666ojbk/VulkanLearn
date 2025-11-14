#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

#include "Data/VulkanConfig.h"

class VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

	static Ref<VulkanContext> Create();
	void Init();

	static Ref<VulkanContext> Get();
	VulkanSwapChain& GetSwapChain() const;

	static VkInstance GetInstance() { return s_VulkanInstance; }
	Ref<VulkanDevice> GetDevice() const { return m_Device; }
	VkDevice GetCurrentDevice() const { return m_Device->GetVulkanDevice(); }
	Ref<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_PhysicalDevice; }
	VulkanConfig& GetConfig() { return s_Config; }
private:
	bool CheckValidationLayerSupport();

	Ref<VulkanDevice> m_Device;
	Ref<VulkanPhysicalDevice> m_PhysicalDevice;

	VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;

	inline static VkInstance s_VulkanInstance;

	VulkanConfig s_Config;
};