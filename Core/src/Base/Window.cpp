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

	uint32_t width = props.Width;
	uint32_t height = props.Height;

	m_Window = glfwCreateWindow(width, height, props.Title.c_str(), nullptr, nullptr);

	// 创建渲染上下文
	m_RendererContext = VulkanContext::Create();
	m_RendererContext->Init();

	m_SwapChain = new VulkanSwapChain();
	m_SwapChain->Init(VulkanContext::GetInstance(), m_RendererContext->GetDevice());
	m_SwapChain->InitSurface(m_Window);
	m_SwapChain->Create(&width, &height);
}

void Window::Shutdown()
{
	m_SwapChain->Destroy();
	delete m_SwapChain;

	m_RendererContext->GetDevice()->Destroy();

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}
