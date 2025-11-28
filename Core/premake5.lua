project "Core"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"
	
	filter "system:windows"
		systemversion "latest"
		staticruntime "On"
		entrypoint "mainCRTStartup"
		linkoptions { "/SUBSYSTEM:CONSOLE" }
		buildoptions { "/utf-8" }

	files
	{
		"src/**.h",
		"src/**.cpp",
		"../vendor/stb/include/**.h"
	}
	defines
	{
		"GLFW_INCLUDE_NONE"
	}
	includedirs
	{
		"src",
		"../vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.stb_image}"
	}
	links
	{
		"GLFW",
		"%{Library.Vulkan}"
	}

	filter "configurations:Debug"
		runtime "Debug"
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}