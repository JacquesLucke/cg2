#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../parametric_surface_viewer.hpp"

bool ParametricSurfaceViewer::onSetup() {
    return true;
}

void ParametricSurfaceViewer::onRenderUI() {
    ImGui::Text("Hello World");
}