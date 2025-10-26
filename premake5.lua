include "Dependencies.lua"

workspace "VulkanLearn"
	architecture "x64"
	startproject "Core"

	configurations
	{
		"Debug",
		"Release",
	}
	defines
	{
		"SPDLOG_USE_STD_FORMAT",
	}
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Vendor/GLFW"
group ""

include "Core"
include "Sandbox"