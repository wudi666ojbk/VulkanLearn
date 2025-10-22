#include "pch.h"
#include "VulkanSwapChain.h"

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
}

void VulkanSwapChain::Destroy()
{
	auto device = m_Device->GetVulkanDevice();
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDeviceWaitIdle(device);
}