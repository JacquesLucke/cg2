#define _USE_MATH_DEFINES
#include <math.h>

#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../parametric_surface_viewer.hpp"
#include "../mesh_utils.hpp"

bool ParametricSurfaceViewer::onSetup() {
    flatShader = new FlatShader();

    OffFileData *offData = loadRelOffResource("franke4.off");
    assert(offData != nullptr);
    sourcePoints = offData->positions;
    delete offData;

    sourcePointsCloud = new PointCloudMesh<VertexP>(createVertexPVector(sourcePoints));

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
        float offset = i / (divX - 1) * scale;
        vertices.push_back(VertexP(glm::vec3(offset, 0,     0)));
        vertices.push_back(VertexP(glm::vec3(offset, 0, scale)));
    }
    for (float i = 0; i < divZ; i++) {
        float offset = i / (divZ - 1) * scale;
        vertices.push_back(VertexP(glm::vec3(    0, 0, offset)));
        vertices.push_back(VertexP(glm::vec3(scale, 0, offset)));
    }
    return new LinesMesh<VertexP>(vertices);
}

void ParametricSurfaceViewer::onUpdate() {
    if (!camera->isFlying() && isKeyDown(GLFW_KEY_F)) {
        camera->enableFlyMode();
    }
    if (camera->isFlying() && isKeyDown(GLFW_KEY_ESCAPE)) {
        camera->disableFlyMode();
    }

    camera->update();
}

void ParametricSurfaceViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();
    drawGrid();
    drawSourcePoints();
}

void ParametricSurfaceViewer::prepareDrawDimensions() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();
}

void ParametricSurfaceViewer::setViewProjMatrixInShaders() {
    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    flatShader->bind();
    flatShader->setViewProj(matViewProj);
}

void ParametricSurfaceViewer::drawGrid() {
    if (gridLinesMesh == nullptr) {
        gridLinesMesh = generateGridLinesMesh(xDivisions, zDivisions, baseGridSize);
    }

    flatShader->bind();
    flatShader->resetModelMatrix();
    flatShader->setColor(1, 1, 1);
    gridLinesMesh->bindBuffers(flatShader);
    gridLinesMesh->draw();
}

void ParametricSurfaceViewer::drawSourcePoints() {
    flatShader->bind();
    flatShader->setColor(1, 0, 0);
    glm::mat4 changeYandZMatrix = glm::mat4(
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1);
    flatShader->setModelMatrix(changeYandZMatrix);
    sourcePointsCloud->bindBuffers(flatShader);
    sourcePointsCloud->draw();
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