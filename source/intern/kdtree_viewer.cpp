#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/glm.hpp>

#include "../camera.hpp"
#include "../kdtree_viewer.hpp"

bool KDTreeViewer::onSetup() {
    offData = readOffFile("/home/jacques/cg2-git/cg2/resources/teapot.off");
    assert(offData != nullptr);

    shader = GLProgram::FromFile("/home/jacques/cg2-git/cg2/resources/default.shader");
    assert(shader != nullptr);
    shader->compile();

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, offData->positions.size() * sizeof(Vector<3>), offData->positions.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(0);

    return true;
}

void KDTreeViewer::onRender() {
    shader->bind();
    shader->setUniform4f("u_Color", color);

    PerspectiveCamera camera(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 1, window()->aspect(), 0.1, 200);
    shader->setUniformMat4f("u_MVP", camera.getViewProjectionMatrix());

    glPointSize(2);
    glDrawArrays(GL_POINTS, 0, offData->positions.size());
}

void KDTreeViewer::onRenderUI() {
    ImGui::ColorEdit3("Color", color);
}