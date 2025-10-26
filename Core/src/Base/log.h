#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

class Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
private:
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

// 初始化日志系统
#define CORE_LOG_INIT() ::Log::Init()

// 日志宏定义
#define CORE_TRACE(...) ::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CORE_INFO(...)  ::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_WARN(...)  ::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_ERROR(...) ::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CORE_FATAL(...) ::Log::GetCoreLogger()->critical(__VA_ARGS__)

// 断言宏定义
#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x
#define INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { CORE_ERROR(msg, __VA_ARGS__); __debugbreak(); } }
#define INTERNAL_ASSERT_WITH_MSG(type, check, ...) INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define INTERNAL_ASSERT_NO_MSG(type, check) INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define INTERNAL_ASSERT_GET_MACRO(...) EXPAND_MACRO( INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, INTERNAL_ASSERT_WITH_MSG, INTERNAL_ASSERT_NO_MSG) )
#define CORE_ASSERT(...) EXPAND_MACRO( INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )