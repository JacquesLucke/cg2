#include <window.hpp>

#include <iostream>

namespace cgX
{
    Window::Window()
        : window(nullptr) { }

    Window::~Window() {
      terminate();
    }

    bool Window::setup(const std::string& name, const Config& config) {
        if(!glfwInit())
            return false;

        window = glfwCreateWindow(config.xRes, config.yRes, name.c_str(), nullptr, nullptr);

        if(nullptr == window) {
            terminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

        return true;
     }

    void Window::terminate() {
        glfwDestroyWindow(window);
        window = nullptr;
        glfwTerminate();
    }

    bool Window::good() const {
        return nullptr != window;
    }

    bool Window::shouldClose() const {
        return !good() || glfwWindowShouldClose(window);
    }

    void Window::beginFrame() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::endFrame() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
