#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"

typedef float (*ImplicitSurfaceFunction)(glm::vec3 &position);

TriangleMesh<VertexP> *meshImplicitSurface(ImplicitSurfaceFunction f) {
    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;

    positions.push_back(glm::vec3(0, 0, 0));
    positions.push_back(glm::vec3(1, 0, 0));
    positions.push_back(glm::vec3(0, 1, 0));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    return new TriangleMesh<VertexP>(createVertexPVector(positions), indices);
}

bool ImplicitSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    surface = meshImplicitSurface(nullptr);
    return true;
}

void ImplicitSurfaceViewer::onUpdate() {
    camera->update(GLFW_KEY_F, getElapsedMilliseconds());
}

void ImplicitSurfaceViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();
    drawSurface();
}

void ImplicitSurfaceViewer::prepareDrawDimensions() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();
}

void ImplicitSurfaceViewer::setViewProjMatrixInShaders() {
    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    flatShader->bind();
    flatShader->setViewProj(matViewProj);
}

void ImplicitSurfaceViewer::drawSurface() {
    flatShader->bind();
    flatShader->resetModelMatrix();
    flatShader->setColor(1, 0, 0);
    surface->bindBuffers(flatShader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    surface->draw();
}

void ImplicitSurfaceViewer::onRenderUI() {
    if (camera->isFlying()) {
        ImGui::Text("Disable fly mode with ESC");
        return;
    }

    ImGui::SliderFloat("Radius", &radius, 0.0, 2.0);
}