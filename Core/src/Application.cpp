#include "pch.h"
#include "Application.h"

#include "Base/Window.h"
#include <GLFW/glfw3.h>

#include "Renderer/VulkanContext.h"

Application* Application::s_Instance = nullptr;

Application::Application()
{
	// 设置控制台输出为UTF-8编码
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	// 初始化日志系统
	Log::Init();

	m_Window = CreateScope<Window>();
	m_Window->Init();
}

Application::~Application()
{
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_Window->GetNativeWindow()))
	{
        glfwPollEvents();
	}
}

void Application::Close()
{
	m_Running = false;
}