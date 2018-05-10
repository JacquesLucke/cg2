#include <iostream>
#include "../window.hpp"

Window::Window(GLFWwindow* handle)
    : _handle(handle) { }

Window::~Window() {
    glfwDestroyWindow(_handle);
}

void Window::activateContext() {
    glfwMakeContextCurrent(_handle);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(_handle);
}

void Window::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::endFrame() {
    glfwSwapBuffers(_handle);
}

float Window::aspect() {
    int width, height;
    glfwGetWindowSize(_handle, &width, &height);
    return (float)width / (float)height;
}

int Window::width() {
    int width;
    glfwGetWindowSize(_handle, &width, nullptr);
    return width;
}

int Window::height() {
    int height;
    glfwGetWindowSize(_handle, nullptr, &height);
    return height;
}