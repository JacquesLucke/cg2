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

LinesMesh<VertexP> *generateGridLinesMesh(int divX, int divZ, float scale) {
    std::vector<VertexP> vertices;
    for (float i = 0; i < divX; i++) {
        float offset = i / (divX - 1) * 2 * scale - scale;
        vertices.push_back(VertexP(glm::vec3(offset, 0, -scale)));
        vertices.push_back(VertexP(glm::vec3(offset, 0, +scale)));
    }
    for (float i = 0; i < divZ; i++) {
        float offset = i / (divZ - 1) * 2 * scale - scale;
        vertices.push_back(VertexP(glm::vec3(-scale, 0, offset)));
        vertices.push_back(VertexP(glm::vec3(+scale, 0, offset)));
    }
    return new LinesMesh<VertexP>(vertices);
}


void ParametricSurfaceViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();
    drawGrid();
}

void ParametricSurfaceViewer::prepareDrawDimensions() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();
}

void ParametricSurfaceViewer::setViewProjMatrixInShaders() {
    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    gridShader->bind();
    gridShader->setTransforms(matViewProj);
}

void ParametricSurfaceViewer::drawGrid() {
    if (gridLinesMesh == nullptr) {
        gridLinesMesh = generateGridLinesMesh(xDivisions, zDivisions, baseGridSize);
    }

    gridShader->bind();
    gridShader->setColor(1, 1, 1);
    gridLinesMesh->bindBuffers(gridShader);
    gridLinesMesh->draw();
}

void ParametricSurfaceViewer::onRenderUI() {
    bool gridChanged = false;
    gridChanged |= ImGui::SliderFloat("Base Grid Size", &baseGridSize, 0.1f, 10.0f);
    gridChanged |= ImGui::SliderInt("X Divisions", &xDivisions, 2, 30);
    gridChanged |= ImGui::SliderInt("Z Divisions", &zDivisions, 2, 30);

    if (gridChanged) {
        resetGrid();
    }
}

void ParametricSurfaceViewer::resetGrid() {
    if (gridLinesMesh != nullptr) {
        delete gridLinesMesh;
        gridLinesMesh = nullptr;
    }
}