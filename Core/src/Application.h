#pragma once
#include "Base/Base.h"
#include <vulkan/vulkan.h>

#include "Base/Window.h"

class Application
{
public:
	Application();
	~Application();

	void Run();
	void Close();

	static inline Application& Get() { return *s_Instance; }
	inline Window& GetWindow() { return *m_Window; }
private:
	static Application* s_Instance;

	Scope<Window> m_Window;

	bool m_Running = true;
};