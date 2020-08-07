#include "timer.h"
#include <iostream>
#include <GLFW/glfw3.h>

FuncTimer::FuncTimer(const char* name)
	: m_Name(name)
{
	m_Start = std::chrono::high_resolution_clock::now();
}

FuncTimer::~FuncTimer()
{
	auto end = std::chrono::high_resolution_clock::now();
	// only prints in ms
	std::cout << m_Name << " took " << std::chrono::duration_cast<std::chrono::milliseconds>(end-m_Start).count() << " ms" << std::endl;
}

FpsTimer::FpsTimer()
: m_Frames(0)
{
	m_Lasttime = glfwGetTime();
}

void FpsTimer::Update()
{
	m_Frames++;
	double current = glfwGetTime();
	if (current - m_Lasttime > 1.0) {
		std::cout << "FPS: " << m_Frames << '\n';
		m_Lasttime = current;
		m_Frames = 0;
	}
}