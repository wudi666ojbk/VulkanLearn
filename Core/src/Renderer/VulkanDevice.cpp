#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
VulkanPhysicalDevice::VulkanPhysicalDevice()
{
    auto instance = VulkanContext::GetInstance();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicaldevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicaldevices.data());

    for (const auto& physicaldevice : physicaldevices) 
    {
        if (IsDeviceSuitable(physicaldevice)) 
        {
            m_PhysicalDevice = physicaldevice;
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Create()
{
	return CreateRef<VulkanPhysicalDevice>();
}

bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice physicaldevice)
{
    return true;
}

VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
	: m_PhysicalDevice(physicalDevice), m_EnabledFeatures(enabledFeatures)
{
}

VulkanDevice::~VulkanDevice()
{
}
