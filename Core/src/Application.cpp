#include "pch.h"
#include "Application.h"

#include "Base/Window.h"
#include <GLFW/glfw3.h>

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanShader.h"
#include "Renderer/VulkanPipeline.h"
#include "Renderer/VulkanRenderer.h"

Application* Application::s_Instance = nullptr;

Application::Application()
{
	s_Instance = this;
	// 设置控制台输出为UTF-8编码
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	// 初始化日志系统
	Log::Init();

	m_Window = CreateScope<Window>();
	m_Window->Init();

	auto& swap = m_Window->GetSwapChain();
	auto shader = VulkanShader::Init();
	auto device = VulkanContext::Get()->GetCurrentDevice();
	Ref <VulkanPipeline> pipeline = VulkanPipeline::Create(shader, &swap);
	m_Renderer = CreateScope<VulkanRenderer>();
	m_Renderer->Init(pipeline);
}

Application::~Application()
{
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_Window->GetNativeWindow()))
	{
		m_Renderer->DrawFrame();
        glfwPollEvents();
	}

	vkDeviceWaitIdle(VulkanContext::Get()->GetCurrentDevice());
}

void Application::Close()
{
	m_Running = false;
}