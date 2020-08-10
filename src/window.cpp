#include "window.h"
#include <iostream>

Window::Window(int width, int height, const char* title)
{
    if (!glfwInit()) {
        std::cout << "glfwInitialization Failed!" << std::endl;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // NOTE: this transparency should be part of some configurable option.
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    if (!(m_Window = glfwCreateWindow(width, height, title, NULL, NULL))) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        glfwTerminate();
        return;
    }
    glfwSwapInterval(1);
    glViewport(0, 0, width, height);
}

Window::~Window()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::SetResizeCb(GLFWframebuffersizefun cb)
{
    glfwSetFramebufferSizeCallback(m_Window, cb);
}

void Window::SetSwapInterval(int i)
{
    glfwSwapInterval(i);
}

void Window::Update()
{
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}