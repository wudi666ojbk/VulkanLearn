#include "pch.h"
#include "Vulkan.h"

void VulkanCheckResult(VkResult result, const char* file, int line) {
    if (result != VK_SUCCESS) {
        printf("Vulkan error: %d in file %s at line %d\n", result, file, line);
        abort();
    }
}