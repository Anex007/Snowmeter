#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();

    void SetResizeCb(GLFWframebuffersizefun cb);
    void SetSwapInterval(int i);
    inline bool Running() { return !glfwWindowShouldClose(m_Window); }
    void Update();
private:
    GLFWwindow* m_Window = nullptr;
};