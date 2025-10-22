#include "pch.h"
#include "Window.h"

Window::~Window()
{
	Shutdown();
}

void Window::Init()
{
	// 窗口初始化
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	WindowProps props("VulkanLearn");
	m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), nullptr, nullptr);

	// 创建渲染上下文
	m_RendererContext = VulkanContext::Create();
	m_RendererContext->Init();
}

void Window::Shutdown()
{
	m_RendererContext->GetDevice()->Destroy();

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}
