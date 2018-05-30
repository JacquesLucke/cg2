#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../parametric_surface_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../bezier_curve.hpp"
#include "../timer.hpp"


bool ParametricSurfaceViewer::onSetup() {
    flatShader = new FlatShader();

    OffFileData *offData = loadRelOffResource("franke5.off");
    assert(offData != nullptr);
    sourcePoints = offData->positions;
    delete offData;

    kdTree = new KDTreeVec3_2D(sourcePoints.data(), sourcePoints.size(), 5);
    kdTree->balance();

    sourcePointsCloud = new PointCloudMesh<VertexP>(createVertexPVector(sourcePoints));
    boundingBox = findBoundingBox<glm::vec3, 3>(sourcePoints);
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

glm::mat4 changeYandZMatrix(
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
);

void ParametricSurfaceViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();

    if (useDepthTest) glEnable(GL_DEPTH_TEST);

    if (displayGrid) drawGrid();
    if (displaySourcePoints) drawSourcePoints();
    if (displaySurface) drawSurface();
    if (displayNormals && finalSurfaceType == FinalSurfaceType::MLS) drawSurfaceNormals();
    if (displayBezierBase && finalSurfaceType == FinalSurfaceType::Bezier) drawBezierBase();

    if (useDepthTest) glDisable(GL_DEPTH_TEST);
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
    flatShader->setModelMatrix(changeYandZMatrix);
    flatShader->setColor(0.3f, 0.3f, 0.3f);
    gridLinesMesh->bindBuffers(flatShader);
    gridLinesMesh->draw();
}

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

void ParametricSurfaceViewer::drawBezierBase() {
    flatShader->bind();
    flatShader->setModelMatrix(changeYandZMatrix);
    flatShader->setColor(0.0, 1.0, 0.0);
    glLineWidth(2);
    bezierBaseSurface->bindBuffers(flatShader);
    bezierBaseSurface->draw();
    glLineWidth(1);
}

void ParametricSurfaceViewer::onRenderUI() {
    bool settingChanged = false;
    ImGui::Checkbox("Display Grid", &displayGrid);
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Generated Mesh", &displaySurface);
    ImGui::Checkbox("Display Normals", &displayNormals);

    if (displaySurface | displayNormals) {
        ImGui::Separator();
        ImGui::Text("Base Grid");

        settingChanged |= ImGui::SliderInt("U Divisions", &uDivisions, 2, 30);
        settingChanged |= ImGui::SliderInt("V Divisions", &vDivisions, 2, 30);
        settingChanged |= ImGui::SliderFloat("Radius", &weightRadius, 0.01f, 1.0f);

        settingChanged |= ImGui::RadioButton("SVD", (int*)&leastSquaresSolver, LeastSquaresSolver::SVD); ImGui::SameLine();
        settingChanged |= ImGui::RadioButton("QR", (int*)&leastSquaresSolver, LeastSquaresSolver::QR); ImGui::SameLine();
        settingChanged |= ImGui::RadioButton("Normal", (int*)&leastSquaresSolver, LeastSquaresSolver::Normal);

        settingChanged |= ImGui::Checkbox("Parallel Surface Generation", &parallelSurfaceGeneration);

        ImGui::Separator();
        settingChanged |= ImGui::RadioButton("Subdivide MLS", (int*)&finalSurfaceType, FinalSurfaceType::MLS); ImGui::SameLine();
        settingChanged |= ImGui::RadioButton("Bezier Surface", (int*)&finalSurfaceType, FinalSurfaceType::Bezier);

        settingChanged |= ImGui::InputInt("Subdivision Level", &subdivisionLevel, 1, 1, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
        subdivisionLevel = std::min(std::max(subdivisionLevel, 0), 5);
        if (finalSurfaceType == FinalSurfaceType::Bezier) {
            ImGui::Checkbox("Display Base Grid", &displayBezierBase);
        }
    }

    ImGui::Separator();
    ImGui::Text("Draw Settings");
    ImGui::SliderInt("Point Size", &sourcePointSize, 1, 10);
    settingChanged |= ImGui::SliderFloat("Normals Length", &normalsLength, 0.0f, 0.3f);
    ImGui::Checkbox("Depth Test", &useDepthTest);

    if (settingChanged) {
        updateGeneratedData();
    }
}

void ParametricSurfaceViewer::updateGeneratedData() {
    TIMEIT("update generated data")

    deleteGeneratedData();
    createGrid();
    if (finalSurfaceType == FinalSurfaceType::MLS){
        createSurfaceAndNormals_MLS();
    } else if (finalSurfaceType == FinalSurfaceType::Bezier) {
        createSurfaceAndNormals_Bezier();
    }
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
    if (bezierBaseSurface != nullptr) {
        delete bezierBaseSurface;
        bezierBaseSurface = nullptr;
    }
}

void ParametricSurfaceViewer::createGrid() {
    gridLinesMesh = generateXYGridLinesMesh(uDivisions, vDivisions, boundingBox);
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

int subdivideToDivisions(int divisions, int subdivisionLevel) {
    return divisions + (divisions - 1) * (int)pow(2, subdivisionLevel - 1);
}

void ParametricSurfaceViewer::createSurfaceAndNormals_MLS() {
    int xDiv = subdivideToDivisions(uDivisions, subdivisionLevel);
    int yDiv = subdivideToDivisions(vDivisions, subdivisionLevel);

    std::vector<glm::vec3> points = calcXYGridPoints(xDiv, yDiv, boundingBox);
    std::vector<EdgeIndices> edges = calcGridEdges(xDiv, yDiv);
    std::vector<glm::vec3> normals(points.size());

    setDataWithMovingLeastSquares(points, normals, kdTree, weightRadius,
        leastSquaresSolver, parallelSurfaceGeneration);

    resultingSurface = new WireframeMesh<VertexP>(createVertexPVector(points), edges);
    surfaceNormalLines = createLineSegmentsMesh(points, normals, normalsLength);
}


std::vector<BezierCurve> calcSurfaceSegments(std::vector<glm::vec3> gridPoints, int stride) {
    std::vector<BezierCurve> curves;
    for (unsigned int i = 0; i < gridPoints.size() / stride; i++) {
        auto first = gridPoints.begin() + stride * i;
        auto last = gridPoints.begin() + stride * (i + 1);
        BezierCurve curve(std::vector<glm::vec3>(first, last));
        curves.push_back(curve);
    }
    return curves;
}

std::vector<glm::vec3> calcBezierSurface(std::vector<glm::vec3> gridPoints, int stride, int uDivisions, int vDivisions) {
    auto curves = calcSurfaceSegments(gridPoints, stride);

    std::vector<glm::vec3> surfacePoints;
    for (int i = 0; i < vDivisions; i++) {
        float t = i / (vDivisions - 1.0f);
        std::vector<glm::vec3> controls = evaluateMultipleBezierCurves(curves, t);
        auto curvePoints = BezierCurve(controls).getPositionSamples(uDivisions);
        surfacePoints.insert(surfacePoints.end(), curvePoints.begin(), curvePoints.end());
    }
    return surfacePoints;
}

void ParametricSurfaceViewer::createSurfaceAndNormals_Bezier() {
    int xBaseDiv = uDivisions;
    int yBaseDiv = vDivisions;

    std::vector<glm::vec3> basePoints = calcXYGridPoints(xBaseDiv, yBaseDiv, boundingBox);
    std::vector<glm::vec3> dummy(basePoints.size());

    setDataWithMovingLeastSquares(basePoints, dummy, kdTree, weightRadius,
        leastSquaresSolver, parallelSurfaceGeneration);

    int xDiv = subdivideToDivisions(xBaseDiv, subdivisionLevel);
    int yDiv = subdivideToDivisions(yBaseDiv, subdivisionLevel);

    auto surfacePoints = calcBezierSurface(basePoints, yBaseDiv, xDiv, yDiv);
    auto edges = calcGridEdges(yDiv, xDiv);
    resultingSurface = new WireframeMesh<VertexP>(createVertexPVector(surfacePoints), edges);

    auto baseSurfaceEdges = calcGridEdges(xBaseDiv, yBaseDiv);
    bezierBaseSurface = new WireframeMesh<VertexP>(createVertexPVector(basePoints), baseSurfaceEdges);
}