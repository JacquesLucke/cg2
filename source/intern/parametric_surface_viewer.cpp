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

    updateGeneratedData();

    return true;
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

    if (displaySourcePoints) drawSourcePoints();
    if (displaySurface) drawSurface();
    if (displayNormals) drawSurfaceNormals();
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

void ParametricSurfaceViewer::drawSurface() {
    flatShader->bind();
    flatShader->setModelMatrix(changeYandZMatrix);
    flatShader->setColor(1, 1, 0);
    resultingSurface->bindBuffers(flatShader);
    resultingSurface->draw();
}

void ParametricSurfaceViewer::drawSurfaceNormals() {
    flatShader->bind();
    flatShader->setModelMatrix(changeYandZMatrix);
    flatShader->setColor(0.5, 0.5, 0);
    surfaceNormalLines->bindBuffers(flatShader);
    surfaceNormalLines->draw();
}

void ParametricSurfaceViewer::onRenderUI() {
    bool settingChanged = false;
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Generated Mesh", &displaySurface);
    ImGui::Checkbox("Display Normals", &displayNormals);

    if (displaySurface | displayNormals) {
        settingChanged |= ImGui::SliderInt("X Divisions", &xDivisions, 2, 30);
        settingChanged |= ImGui::SliderInt("Z Divisions", &zDivisions, 2, 30);
        settingChanged |= ImGui::SliderFloat("Radius", &weightRadius, 0.01f, 1.0f);

        settingChanged |= ImGui::Checkbox("Parallel Surface Generation", &parallelSurfaceGeneration);
        settingChanged |= ImGui::SliderFloat("Normals Length", &normalsLength, 0.0f, 0.3f);

        settingChanged |= ImGui::RadioButton("SVD", (int*)&leastSquaresSolver, LeastSquaresSolver::SVD); ImGui::SameLine();
        settingChanged |= ImGui::RadioButton("QR", (int*)&leastSquaresSolver, LeastSquaresSolver::QR); ImGui::SameLine();
        settingChanged |= ImGui::RadioButton("Normal", (int*)&leastSquaresSolver, LeastSquaresSolver::Normal);
    }

    ImGui::SliderInt("Point Size", &sourcePointSize, 1, 10);

    if (settingChanged) {
        updateGeneratedData();
    }
}

void ParametricSurfaceViewer::updateGeneratedData() {
    deleteGeneratedData();
    createGrid();
    createSurfaceAndNormals();
}

void ParametricSurfaceViewer::deleteGeneratedData() {
    if (gridLinesMesh != nullptr) {
        delete gridLinesMesh;
        gridLinesMesh = nullptr;
    }
    if (resultingSurface != nullptr) {
        delete resultingSurface;
        resultingSurface = nullptr;
    }
    if (surfaceNormalLines != nullptr) {
        delete surfaceNormalLines;
        surfaceNormalLines = nullptr;
    }
}

void ParametricSurfaceViewer::createGrid() {
    gridLinesMesh = generateXZGridLinesMesh(xDivisions, zDivisions, baseGridSize);
}

LinesMesh<VertexP> *createLineSegmentsMesh(std::vector<glm::vec3> starts, std::vector<glm::vec3> offsets, float scale) {
    assert(starts.size() == offsets.size());

    std::vector<glm::vec3> linePoints;
    for (unsigned int i = 0; i < starts.size(); i++) {
        linePoints.push_back(starts[i]);
        linePoints.push_back(starts[i] + offsets[i] * scale);
    }
    return new LinesMesh<VertexP>(createVertexPVector(linePoints));
}

void ParametricSurfaceViewer::createSurfaceAndNormals() {
    std::vector<glm::vec3> points = calcXYGridPoints(xDivisions, zDivisions, baseGridSize);
    std::vector<EdgeIndices> edges = calcGridEdges(xDivisions, zDivisions);
    std::vector<glm::vec3> normals(points.size());

    setDataWithMovingLeastSquares(points, normals, kdTree, weightRadius,
        leastSquaresSolver, parallelSurfaceGeneration);

    resultingSurface = new WireframeMesh<VertexP>(createVertexPVector(points), edges);
    surfaceNormalLines = createLineSegmentsMesh(points, normals, normalsLength);
}