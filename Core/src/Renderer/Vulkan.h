#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)\
{\
	VkResult res = (f);\
	VulkanCheckResult(res, __FILE__, __LINE__);\
}

inline std::string VKResultToString(VkResult result);

void VulkanCheckResult(VkResult result, const char* file, int line);
void VulkanCheckResult(VkResult result);