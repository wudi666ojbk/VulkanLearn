#pragma once
#include "Vulkan.h"

class VulkanPhysicalDevice
{
public:

	struct QueueFamilyIndices
	{
		int32_t Graphics = -1;
		int32_t Compute = -1;
		int32_t Transfer = -1;
	};

	VulkanPhysicalDevice();		// 创建物理设备

	static Ref<VulkanPhysicalDevice> Create();

	bool IsDeviceSuitable(VkPhysicalDevice device);

	VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
	const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
private:
	QueueFamilyIndices GetQueueFamilyIndices(int queueFlags);
private:
	VkPhysicalDevice m_PhysicalDevice = nullptr;
	VkPhysicalDeviceProperties m_Properties;

	QueueFamilyIndices m_QueueFamilyIndices;
	std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
	std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

	friend class VulkanDevice;
};

class VulkanCommandPool
{

};

class VulkanDevice
{
public:
	VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);			// 创建逻辑设备
	~VulkanDevice();

	void Destroy();

	const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }
	VkDevice GetVulkanDevice() const { return m_LogicalDevice; }
private:
	VkDevice m_LogicalDevice = nullptr;
	Ref<VulkanPhysicalDevice> m_PhysicalDevice;
	VkPhysicalDeviceFeatures m_EnabledFeatures;
};

