#pragma once
#include "Vulkan.h"

#include "VulkanCommandPool.h"
#include <map>

struct QueueFamilyIndices
{
	int32_t Graphics = -1;
	int32_t Compute = -1;
	int32_t Transfer = -1;
};

class VulkanPhysicalDevice
{
public:
	VulkanPhysicalDevice();		// 创建物理设备

	static Ref<VulkanPhysicalDevice> Create();

	bool IsExtensionSupported(const std::string& extensionName) const;

	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

	VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
	const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
	const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_MemoryProperties; }
private:
	QueueFamilyIndices GetQueueFamilyIndices(int queueFlags);
	// 枚举支持的扩展并保存到m_SupportedExtensions中
	void EnumerateSupportedExtensions();
private:
	VkPhysicalDevice m_PhysicalDevice = nullptr;
	VkPhysicalDeviceProperties m_Properties;

	VkPhysicalDeviceFeatures m_Features;
	VkPhysicalDeviceMemoryProperties m_MemoryProperties;

	QueueFamilyIndices m_QueueFamilyIndices;
	std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
	std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

	// 设备支持的扩展
	std::unordered_set<std::string> m_SupportedExtensions;

	friend class VulkanDevice;
};

class VulkanDevice
{
public:
	VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);			// 创建逻辑设备
	~VulkanDevice();

	void Destroy();

	VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
	VkQueue GetComputeQueue() { return m_ComputeQueue; }

	VkCommandBuffer GetCommandBuffer(bool begin, bool compute = false);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer);

	const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }
	VkDevice GetVulkanDevice() const { return m_LogicalDevice; }
private:
	Ref<VulkanCommandPool> GetThreadLocalCommandPool();
	Ref<VulkanCommandPool> GetOrCreateThreadLocalCommandPool();
private:
	VkDevice m_LogicalDevice = nullptr;
	Ref<VulkanPhysicalDevice> m_PhysicalDevice;
	VkPhysicalDeviceFeatures m_EnabledFeatures;

	std::map<std::thread::id, Ref<VulkanCommandPool>> m_CommandPools;

	VkQueue m_GraphicsQueue;
	VkQueue m_ComputeQueue;
};

