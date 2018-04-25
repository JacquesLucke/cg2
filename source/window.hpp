#pragma once

#include <string>
#include <GLFW/glfw3.h>

class Window {
public:
    static Window *Window::TryCreateNew(const std::string& title, int width, int height){
        GLFWwindow* handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (handle == nullptr) {
            return nullptr;
        } else {
            return new Window(handle);
        }
    }

    Window(GLFWwindow* handle);
    ~Window();

    void activateContext();
    bool shouldClose() const;
    void beginFrame();
    void endFrame();

    float aspect();
    int width();
    int height();

    GLFWwindow* handle() { return _handle; }

private:
    GLFWwindow* _handle;
};

