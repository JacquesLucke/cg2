#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../camera.hpp"
#include "../kdtree_viewer.hpp"
#include "../resources.hpp"

bool KDTreeViewer::onSetup() {
    mesh = loadRelMeshResource<VertexP>("teapot.off");
    assert(mesh != nullptr);

    shader = loadRelShaderResource("default.shader");
    assert(shader != nullptr);
    shader->compile();

    window()->setRenderMode(RENDER_MODE::WIREFRAME);

    return true;
}

void KDTreeViewer::onUpdate() {
    if (!camera->isFlying() && isKeyDown(GLFW_KEY_F)) {
        camera->enableFlyMode();
    }
    if (camera->isFlying() && isKeyDown(GLFW_KEY_ESCAPE)) {
        camera->disableFlyMode();
    }

    camera->update();
}

void KDTreeViewer::onRender() {
    int width, height;
    glfwGetFramebufferSize(window()->handle(), &width, &height);
    glViewport(0, 0, width, height);
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();

    shader->bind();
    shader->setUniform4f("u_Color", color);
    shader->setUniformMat4f("u_MVP", camera->camera->getViewProjectionMatrix());

    window()->render(mesh);
}

static void drawFlyCameraControls() {
    ImGui::LabelText("", "ESC: stop fly mode");
    ImGui::LabelText("", "WASDQE : move camera");
    ImGui::LabelText("", "mouse: rotate camera");
    ImGui::LabelText("", "scroll wheel: change speed");
}

void KDTreeViewer::onRenderUI() {
    auto v = getMousePos();
    std::string s = "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
    ImGui::LabelText("", s.c_str());
    if (camera->isFlying()) {
        drawFlyCameraControls();
        return;
    }

    ImGui::LabelText("", "Press F to start fly mode.");
    ImGui::ColorEdit3("Color", color);
}

bool KDTreeViewer::isKeyDown(int key) {
    return glfwGetKey(window()->handle(), key) == GLFW_PRESS;
}
