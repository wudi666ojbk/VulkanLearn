#pragma once
#include "Base/Base.h"
#include <GLFW/glfw3.h>

#include "Renderer/VulkanContext.h"

struct WindowProps
{
	std::string Title;
	unsigned int Width;
	unsigned int Height;

	WindowProps(const std::string& title, 
		unsigned int width = 1280, unsigned int height = 720)
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