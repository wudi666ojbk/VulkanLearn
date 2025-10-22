include "Dependencies.lua"

workspace "VulkanLearn"
	architecture "x64"
	startproject "Core"

	configurations
	{
		"Debug",
		"Release",
	}
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Vendor/GLFW"
group ""

include "Core"
include "Sandbox"