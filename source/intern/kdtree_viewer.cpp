#include <glad/glad.h>
#include <GLFW/glfw3.h>
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

    mesh->bindBuffers();

    glPointSize(2);
    glDrawElements(GL_POINTS, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);

    mesh->unbindBuffers();
}

static void drawFlyCameraControls() {
    ImGui::LabelText("", "ESC: stop fly mode");
    ImGui::LabelText("", "WASDQE : move camera");
    ImGui::LabelText("", "mouse: rotate camera");
    ImGui::LabelText("", "scroll wheel: change speed");
}

void KDTreeViewer::onRenderUI() {
    if (camera->isFlying()) {
        drawFlyCameraControls();
        return;
    }

    ImGui::LabelText("", std::to_string(mesh->getVertexCount()).c_str());

    ImGui::LabelText("", "Press F to start fly mode.");
    ImGui::ColorEdit3("Color", color);
}

bool KDTreeViewer::isKeyDown(int key) {
    return glfwGetKey(window()->handle(), key) == GLFW_PRESS;
}
