#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
VulkanPhysicalDevice::VulkanPhysicalDevice()
{
    auto instance = VulkanContext::GetInstance();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount); 
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()))

    VkPhysicalDevice selectedPhysicalDevice = nullptr;
    for (VkPhysicalDevice physicalDevice : physicalDevices)
    {
        // 检查物理设备是否适合我们的用途
        vkGetPhysicalDeviceProperties(physicalDevice, &m_Properties);
        if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            selectedPhysicalDevice = physicalDevice;
            break;
        }
    }
    if (!selectedPhysicalDevice)
    {
		CORE_WARN("Could not find discrete GPU.");
        selectedPhysicalDevice = physicalDevices.back();
    }

    CORE_ASSERT(selectedPhysicalDevice, "不能找到适合的物理设备!");
    m_PhysicalDevice = selectedPhysicalDevice;   // 成功创建Vulkan物理设备

    // 获取物理设备的队列族属性
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
	CORE_ASSERT(queueFamilyCount != 0, "Physical device has no queue families!");
    m_QueueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());

    // 队列族
    // 所需的队列需要在创建逻辑设备时请求
    // 由于不同 Vulkan 实现的队列族配置可能不同，这个过程可能会有些复杂，尤其是当应用程序
    // 请求不同类型的队列时
    // 获取请求的队列族类型的队列族索引
    // 请注意，这些索引可能会根据不同的实现而重叠
    static const float defaultQueuePriority(0.0f);
    int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT;
    m_QueueFamilyIndices = GetQueueFamilyIndices(requestedQueueTypes);

	// 图形队列
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		m_QueueCreateInfos.push_back(queueInfo);
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

VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamilyIndices(int flags)
{
	QueueFamilyIndices indices;

	// 专用计算队列
	// 尝试找到一个支持计算但不支持图形的队列族索引
	if (flags & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			auto& queueFamilyProperties = m_QueueFamilyProperties[i];
			if ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				indices.Compute = i;
				break;
			}
		}
	}

	// 专用传输队列
	// 尝试找到一个支持传输但不支持图形和计算的队列族索引
	if (flags & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			auto& queueFamilyProperties = m_QueueFamilyProperties[i];
			if ((queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				indices.Transfer = i;
				break;
			}
		}
	}

	// 对于其他队列类型或如果不存在单独的计算队列，返回第一个支持请求标志的队列族索引
	for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
	{
		if ((flags & VK_QUEUE_TRANSFER_BIT) && indices.Transfer == -1)
		{
			if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				indices.Transfer = i;
		}

		if ((flags & VK_QUEUE_COMPUTE_BIT) && indices.Compute == -1)
		{
			if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.Compute = i;
		}

		if (flags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.Graphics = i;
		}
	}

	return indices;
}

VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
	: m_PhysicalDevice(physicalDevice), m_EnabledFeatures(enabledFeatures)
{
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(physicalDevice->m_QueueCreateInfos.size());;
	createInfo.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data();
	createInfo.pEnabledFeatures = &enabledFeatures;

	VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice->GetVulkanPhysicalDevice(), &createInfo, nullptr, &m_LogicalDevice));
	
}

VulkanDevice::~VulkanDevice()
{
}

void VulkanDevice::Destroy()
{
	vkDeviceWaitIdle(m_LogicalDevice);
	vkDestroyDevice(m_LogicalDevice, nullptr);
}
