#include "pch.h"
#include "VulkanContext.h"
#include "Application.h"
#include "Debug/VulkanDebug.h"

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VulkanContext::VulkanContext()
{
}

VulkanContext::~VulkanContext()
{
	if (enableValidationLayers) VulkanDebug::DestroyDebugUtilsMessengerEXT(s_VulkanInstance, m_DebugUtilsMessenger, nullptr);


	vkDestroyInstance(s_VulkanInstance, nullptr);
}

Ref<VulkanContext> VulkanContext::Create()
{
    return CreateRef<VulkanContext>();
}

void VulkanContext::Init()
{
	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanLearn";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Extension info
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		if (!CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		
		VulkanDebug::PopulateDebugMessengerCreateInfo(debugCreateInfo);
		debugCreateInfo.pNext = nullptr;

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// 创建Vulkan实例
	VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &s_VulkanInstance))

	if (enableValidationLayers) {
		VulkanDebug::CreateDebugUtilsMessengerEXT(s_VulkanInstance, &debugCreateInfo, nullptr, &m_DebugUtilsMessenger);
	}

	// 实例化物理设备类
	m_PhysicalDevice = VulkanPhysicalDevice::Create();
	VkPhysicalDeviceFeatures deviceFeatures;

	// 实例化逻辑设备类
	m_Device = CreateRef<VulkanDevice>(m_PhysicalDevice, deviceFeatures);
}

bool VulkanContext::CheckValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

Ref<VulkanContext> VulkanContext::Get()
{
	return Application::Get().GetWindow().GetRenderContext();
}