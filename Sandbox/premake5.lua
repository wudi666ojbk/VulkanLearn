project "Sandbox"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++23"
	
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
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
	}
	includedirs
	{
		"src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.VulkanSDK}"
	}
	links
	{
		"GLFW",
		"%{Library.Vulkan}"
	}