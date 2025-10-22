#include "pch.h"
#include "Application.h"

#include "Base/Window.h"
#include <GLFW/glfw3.h>

#include "Renderer/VulkanContext.h"

Application* Application::s_Instance = nullptr;

Application::Application()
{
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