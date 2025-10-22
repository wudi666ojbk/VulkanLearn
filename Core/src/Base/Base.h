#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <iostream>
#include <memory>

// 唯一指针
template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

// 共享指针
template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

// 日志宏定义
#define CORE_INFO(message) std::cout << "[INFO] " << message << std::endl
#define CORE_WARN(message) std::cout << "[WARN] " << message << std::endl
#define CORE_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "[ASSERT] " << message << std::endl; \
        __debugbreak(); \
    }