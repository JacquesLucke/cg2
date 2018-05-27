#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../parametric_surface_viewer.hpp"
#include "../mesh_utils.hpp"

bool ParametricSurfaceViewer::onSetup() {
    gridShader = new FlatShader();

    return true;
}

std::vector<glm::vec3> *calcGridPoints(int xDiv, int zDiv) {
    auto points = new std::vector<glm::vec3>();

    float xOffset = -(xDiv - 1) / 2.0f;
    float zOffset = -(zDiv - 1) / 2.0f;

    for (float z = 0; z < zDiv; z++) {
        for (float x = 0; x < xDiv; x++) {
            points->push_back(glm::vec3(
                x + xOffset,
                0,
                z + zOffset
            ));
        }
    }

    return points;
}

std::vector<EdgeIndices> *calcGridEdges(int div1, int div2) {
    auto edges = new std::vector<EdgeIndices>();
    for (int i = 0; i < div1; i++) {
        for (int j = 0; j < div2 - 1; j++) {
            edges->push_back(EdgeIndices(
                i * div2 + j + 0,
                i * div2 + j + 1));
        }
    }
    for (int i = 0; i < div2; i++) {
        for (int j = 0; j < div1 - 1; j++) {
            edges->push_back(EdgeIndices(
                (j + 0) * div2 + i,
                (j + 1) * div2 + i));
        }
    }
    return edges;
}

void ParametricSurfaceViewer::onRender() {
    int width, height;
    glfwGetFramebufferSize(window()->handle(), &width, &height);
    glViewport(0, 0, width, height);
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();

    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    gridShader->bind();
    gridShader->setTransforms(matViewProj);

    if (gridPoints == nullptr) {
        gridPoints = calcGridPoints(xDivisions, zDivisions);
    }

    auto vertices = createVertexPVector(*gridPoints);
    auto edges = calcGridEdges(xDivisions, zDivisions);
    Wireframe<VertexP> points(vertices, *edges);

    gridShader->bind();
    glPointSize(3);
    gridShader->setColor(1, 1, 1);
    points.bindBuffers(gridShader);
    points.draw();
}

void ParametricSurfaceViewer::onRenderUI() {
    ImGui::Text("Hello World");
}