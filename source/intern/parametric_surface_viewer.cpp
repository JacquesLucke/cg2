#define _USE_MATH_DEFINES
#include <math.h>

#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../parametric_surface_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../timer.hpp"


bool ParametricSurfaceViewer::onSetup() {
    flatShader = new FlatShader();

    OffFileData *offData = loadRelOffResource("franke5.off");
    assert(offData != nullptr);
    sourcePoints = offData->positions;
    delete offData;

    sourcePointsCloud = new PointCloudMesh<VertexP>(createVertexPVector(sourcePoints));

    kdTree = new KDTreeVec3_2D(sourcePoints.data(), sourcePoints.size(), 5);
    kdTree->balance();

    return true;
}

std::vector<glm::vec3> calcXYGridPoints(int xDiv, int yDiv, float scale) {
    std::vector<glm::vec3> points;

    for (float x = 0; x < xDiv; x++) {
        for (float y = 0; y < yDiv; y++) {
            points.push_back(glm::vec3(
                x / (xDiv - 1) * scale,
                y / (yDiv - 1) * scale,
                0
            ));
        }
    }

    return points;
}

std::vector<EdgeIndices> calcGridEdges(int div1, int div2) {
    std::vector<EdgeIndices> edges;
    for (int i = 0; i < div1; i++) {
        for (int j = 0; j < div2 - 1; j++) {
            edges.push_back(EdgeIndices(
                i * div2 + j + 0,
                i * div2 + j + 1));
        }
    }
    for (int i = 0; i < div2; i++) {
        for (int j = 0; j < div1 - 1; j++) {
            edges.push_back(EdgeIndices(
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

    if (displaySourcePoints) {
        drawSourcePoints();
    }
    if (displayGeneratedMesh) {
        drawResultingSurface();
    }
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
    flatShader->setColor(0.3f, 0.3f, 0.3f);
    gridLinesMesh->bindBuffers(flatShader);
    gridLinesMesh->draw();
}

glm::mat4 changeYandZMatrix(
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
);

void ParametricSurfaceViewer::drawSourcePoints() {
    flatShader->bind();
    flatShader->setColor(1, 0, 0);
    flatShader->setModelMatrix(changeYandZMatrix);
    glPointSize((float)sourcePointSize);
    sourcePointsCloud->bindBuffers(flatShader);
    sourcePointsCloud->draw();
    glPointSize(1);
}

void ParametricSurfaceViewer::drawResultingSurface() {
    if (resultingSurface == nullptr) {
        std::vector<glm::vec3> points = calcXYGridPoints(xDivisions, zDivisions, baseGridSize);
        std::vector<EdgeIndices> edges = calcGridEdges(xDivisions, zDivisions);
        setZValuesWithMovingLeastSquares(points, kdTree, weightRadius, true);
        resultingSurface = new WireframeMesh<VertexP>(createVertexPVector(points), edges);
    }
    flatShader->bind();
    flatShader->setColor(1, 1, 0);
    flatShader->setModelMatrix(changeYandZMatrix);
    resultingSurface->bindBuffers(flatShader);
    resultingSurface->draw();
}

void ParametricSurfaceViewer::onRenderUI() {
    bool settingChanged = false;
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Generated Mesh", &displayGeneratedMesh);

    if (displayGeneratedMesh) {
        settingChanged |= ImGui::SliderInt("X Divisions", &xDivisions, 2, 30);
        settingChanged |= ImGui::SliderInt("Z Divisions", &zDivisions, 2, 30);
        settingChanged |= ImGui::SliderFloat("Radius", &weightRadius, 0.01f, 1.0f);
    }

    ImGui::SliderInt("Point Size", &sourcePointSize, 1, 10);

    if (settingChanged) {
        resetGeneratedData();
    }
}

void ParametricSurfaceViewer::resetGeneratedData() {
    if (gridLinesMesh != nullptr) {
        delete gridLinesMesh;
        gridLinesMesh = nullptr;
    }
    if (resultingSurface != nullptr) {
        delete resultingSurface;
        resultingSurface = nullptr;
    }
}