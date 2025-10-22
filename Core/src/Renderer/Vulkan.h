#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)\
{\
	VkResult res = (f);\
	VulkanCheckResult(res, __FILE__, __LINE__);\
}

void VulkanCheckResult(VkResult result, const char* file, int line);