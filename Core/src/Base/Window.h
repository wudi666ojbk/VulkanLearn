#pragma once
#include "Base/Base.h"
#include <GLFW/glfw3.h>

#include "Renderer/VulkanContext.h"

struct WindowProps
{
	std::string Title;
	uint32_t Width;
	uint32_t Height;

	WindowProps(const std::string& title, 
		uint32_t width = 1280, uint32_t height = 720)
		: Width(width), Height(height), Title(title)
	{
	}
};

class Window
{
public:
	~Window();

	void Init();
	void Shutdown();

	GLFWwindow* GetNativeWindow() const { return m_Window; }

	// Vulkan
	virtual Ref<VulkanContext> GetRenderContext() { return m_RendererContext; }
	VulkanSwapChain& GetSwapChain() { return *m_SwapChain; }
private:
	GLFWwindow* m_Window;

	Ref<VulkanContext> m_RendererContext;

	VulkanSwapChain* m_SwapChain;
};