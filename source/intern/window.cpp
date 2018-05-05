#include <iostream>
#include "../window.hpp"

Window::Window(GLFWwindow* handle)
    : _handle(handle), drawFlag(GL_TRIANGLES) { }

Window::~Window() {
    glfwDestroyWindow(_handle);
}

void Window::setRenderMode(RENDER_MODE mode) {
    if(RENDER_MODE::WIREFRAME == mode) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawFlag = GL_TRIANGLES;
    } else {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	drawFlag = (RENDER_MODE::SOLID == mode) ? GL_TRIANGLES : GL_POINTS;
    }
}

void Window::setPointSize(int pointSize) {
    glPointSize(pointSize);
}

void Window::activateContext() {
    glfwMakeContextCurrent(_handle);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(_handle);
}

void Window::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT);
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

void Window::onRender(unsigned int indexCount) {
    glDrawElements(drawFlag, indexCount, GL_UNSIGNED_INT, 0);
}
