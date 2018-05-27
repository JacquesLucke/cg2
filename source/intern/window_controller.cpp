#include <cassert>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../window_controller.hpp"

bool WindowController::setup() {
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(_window->handle(), true);
    ImGui::StyleColorsDark();
    return onSetup();
}

void WindowController::teardown() {
    onTeardown();
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
}

void WindowController::update() {
    glfwPollEvents();
    onUpdate();
}

void WindowController::render() {
    _window->beginFrame();

    ImGui_ImplGlfwGL3_NewFrame();
    onRender();
    onRenderUI();
    ImGui::Render();

    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    _window->endFrame();
}

bool WindowController::isRunning() const {
    return !_window->shouldClose();
}

glm::vec2 WindowController::getMousePos() {
    double x, y;
    glfwGetCursorPos(window()->handle(), &x, &y);
    return glm::vec2(x, y);
}

bool WindowController::isKeyDown(int key) {
    return glfwGetKey(window()->handle(), key) == GLFW_PRESS;
}