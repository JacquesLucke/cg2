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

glm::vec4 colorFromZ(float z, BoundingBox<3> &box) {
    float t = box.mapBetween0And1(z, 2);
    glm::vec4 c1 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 c2 = glm::vec4(1.0f, 0.6f, 0.0f, 1.0f);
    return c1 * (1-t) + c2 * t;
}

PointCloudMesh<VertexPC> *coloredPointCloud(std::vector<glm::vec3> &positions, BoundingBox<3> &box) {
    std::vector<VertexPC> vertices;
    for (unsigned int i = 0; i < positions.size(); i++) {
        vertices.push_back(VertexPC(positions[i], colorFromZ(positions[i].z, box)));
    }
    return new PointCloudMesh<VertexPC>(vertices);
}

bool ParametricSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    colorShader = new ShadelessColorShader();

    OffFileData *offData = loadRelOffResource("franke5.off");
    assert(offData != nullptr);
    sourcePoints = offData->positions;
    delete offData;

    kdTree = new KDTreeVec3_2D(sourcePoints.data(), sourcePoints.size(), 5);
    kdTree->balance();

    boundingBox = findBoundingBox<glm::vec3, 3>(sourcePoints);
    sourcePointsCloud = coloredPointCloud(sourcePoints, boundingBox);
    updateGeneratedData();
    return true;
}

void ParametricSurfaceViewer::onUpdate() {
    camera->update(GLFW_KEY_F, getElapsedMilliseconds());
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
    colorShader->bind();
    colorShader->setViewProj(matViewProj);
}

void ParametricSurfaceViewer::drawGrid() {
    flatShader->bind();
    flatShader->setModelMatrix(changeYandZMatrix);
    flatShader->setColor(0.3f, 0.3f, 0.3f);
    gridLinesMesh->bindBuffers(flatShader);
    gridLinesMesh->draw();
}

void ParametricSurfaceViewer::drawSourcePoints() {
    colorShader->bind();
    colorShader->setModelMatrix(changeYandZMatrix);
    glPointSize((float)sourcePointSize);
    sourcePointsCloud->bindBuffers(colorShader);
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
    if (camera->isFlying()) {
        ImGui::Text("Disable fly mode with ESC");
        return;
    }

    bool recalc = false;
    ImGui::Checkbox("Display Grid", &displayGrid);
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Generated Mesh", &displaySurface);
    ImGui::Checkbox("Display Normals", &displayNormals);

    if (displaySurface | displayNormals) {
        ImGui::Separator();
        ImGui::Text("Base Grid");

        recalc |= ImGui::SliderInt("U Divisions", &uDivisions, 2, 30);
        recalc |= ImGui::SliderInt("V Divisions", &vDivisions, 2, 30);
        recalc |= ImGui::RadioButton("Radius", (int*)&radiusSelectionInfo.mode, RadiusSelectionMode::Radius); ImGui::SameLine();
        recalc |= ImGui::RadioButton("K Nearest", (int*)&radiusSelectionInfo.mode, RadiusSelectionMode::KNearest);
        if (radiusSelectionInfo.mode == RadiusSelectionMode::Radius) {
            recalc |= ImGui::SliderFloat("Radius", &radiusSelectionInfo.radius, 0.01f, boundingBox.maxsize());
        } else if (radiusSelectionInfo.mode == RadiusSelectionMode::KNearest) {
            recalc |= ImGui::SliderInt("Amount", &radiusSelectionInfo.k, 1, 100);
        }

        recalc |= ImGui::RadioButton("SVD", (int*)&leastSquaresSolver, LeastSquaresSolver::SVD); ImGui::SameLine();
        recalc |= ImGui::RadioButton("QR", (int*)&leastSquaresSolver, LeastSquaresSolver::QR); ImGui::SameLine();
        recalc |= ImGui::RadioButton("Normal", (int*)&leastSquaresSolver, LeastSquaresSolver::Normal);

        recalc |= ImGui::Checkbox("Parallel Surface Generation", &parallelSurfaceGeneration);

        ImGui::Separator();
        recalc |= ImGui::RadioButton("Subdivide MLS", (int*)&finalSurfaceType, FinalSurfaceType::MLS); ImGui::SameLine();
        recalc |= ImGui::RadioButton("Bezier Surface", (int*)&finalSurfaceType, FinalSurfaceType::Bezier);

        recalc |= ImGui::InputInt("Subdivision Level", &subdivisionLevel, 1, 1, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
        subdivisionLevel = std::min(std::max(subdivisionLevel, 0), 5);
        if (finalSurfaceType == FinalSurfaceType::Bezier) {
            ImGui::Checkbox("Display Base Grid", &displayBezierBase);
        }
    }

    ImGui::Separator();
    ImGui::Text("Draw Settings");
    ImGui::SliderInt("Point Size", &sourcePointSize, 1, 10);
    recalc |= ImGui::SliderFloat("Normals Length", &normalsLength, 0.0f, 0.3f);
    ImGui::Checkbox("Depth Test", &useDepthTest);

    if (recalc) {
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

    setDataWithMovingLeastSquares(points, normals, kdTree,
        radiusSelectionInfo, leastSquaresSolver, parallelSurfaceGeneration);

    resultingSurface = new WireframeMesh<VertexP>(createVertexPVector(points), edges);
    surfaceNormalLines = createLineSegmentsMesh(points, normals, normalsLength);
}


std::vector<BezierCurve> bezierCurvesFromGrid_Rows(std::vector<glm::vec3> &gridPoints, int stride) {
    std::vector<BezierCurve> curves;
    for (unsigned int i = 0; i < gridPoints.size() / stride; i++) {
        auto first = gridPoints.begin() + stride * i;
        auto last = gridPoints.begin() + stride * (i + 1);
        BezierCurve curve(std::vector<glm::vec3>(first, last));
        curves.push_back(curve);
    }
    return curves;
}

std::vector<BezierCurve> bezierCurvesFromGrid_Columns(std::vector<glm::vec3> &gridPoints, int stride) {
    std::vector<BezierCurve> curves;
    for (unsigned int i = 0; i < gridPoints.size() / stride; i++) {
        std::vector<glm::vec3> controls;
        for (int j = 0; j < stride; j++) {
            controls.push_back(gridPoints[i * stride + j]);
        }
        curves.push_back(BezierCurve(controls));
    }
    return curves;
}

std::vector<glm::vec3> calcBezierSurface(std::vector<glm::vec3> &gridPoints, int stride, int uDivisions, int vDivisions) {
    auto curves = bezierCurvesFromGrid_Columns(gridPoints, stride);
    return gridFromBezierCurves(curves, vDivisions, uDivisions);
}

void ParametricSurfaceViewer::createSurfaceAndNormals_Bezier() {
    int xBaseDiv = uDivisions;
    int yBaseDiv = vDivisions;

    std::vector<glm::vec3> basePoints = calcXYGridPoints(xBaseDiv, yBaseDiv, boundingBox);
    std::vector<glm::vec3> dummy(basePoints.size());

    setDataWithMovingLeastSquares(basePoints, dummy, kdTree,
        radiusSelectionInfo, leastSquaresSolver, parallelSurfaceGeneration);

    int xDiv = subdivideToDivisions(xBaseDiv, subdivisionLevel);
    int yDiv = subdivideToDivisions(yBaseDiv, subdivisionLevel);

    auto surfacePoints = calcBezierSurface(basePoints, yBaseDiv, xDiv, yDiv);
    auto edges = calcGridEdges(yDiv, xDiv);
    resultingSurface = new WireframeMesh<VertexP>(createVertexPVector(surfacePoints), edges);

    auto baseSurfaceEdges = calcGridEdges(xBaseDiv, yBaseDiv);
    bezierBaseSurface = new WireframeMesh<VertexP>(createVertexPVector(basePoints), baseSurfaceEdges);
}