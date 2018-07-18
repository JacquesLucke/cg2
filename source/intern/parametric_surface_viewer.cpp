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
#include "../window_controller.hpp"

glm::vec4 colorFromZ(float z, BoundingBox<3> &box) {
    float t = box.mapBetween0And1(z, 2);
    glm::vec4 c1 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 c2 = glm::vec4(1.0f, 0.6f, 0.0f, 1.0f);
    return c1 * (1-t) + c2 * t;
}

PointCloudGPUMesh<VertexPC> *coloredPointCloud(std::vector<glm::vec3> &positions, BoundingBox<3> &box) {
    std::vector<VertexPC> vertices;
    for (unsigned int i = 0; i < positions.size(); i++) {
        vertices.push_back(VertexPC(positions[i], colorFromZ(positions[i].z, box)));
    }
    return new PointCloudGPUMesh<VertexPC>(vertices);
}

bool ParametricSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    colorShader = new ShadelessColorShader();
    lightShader = new BlinnPhongShader();

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
    if (!ImGui::GetIO().WantCaptureMouse) {
        camera->update(getElapsedMilliseconds());
    }
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
    if (displayNormals) drawSurfaceNormals();
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
    lightShader->bind();
    lightShader->setViewProj(matViewProj);
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
    if(SurfaceRenderMode::WIREFRAME == surfaceRenderMode) {
        flatShader->bind();
        flatShader->setModelMatrix(changeYandZMatrix);
        flatShader->setColor(1, 1, 0);
        resultingSurfaceWireframe->bindBuffers(flatShader);
        resultingSurfaceWireframe->draw();
    } else {
        lightShader->bind();
        lightShader->setModelMatrix(changeYandZMatrix);
        lightShader->setAmbientColor(0,0,0);
        lightShader->setDiffuseColor(0.8f, 0.8f, 0.2f);
        lightShader->setSpecularColor(1,1,1);
        auto lightPos = glm::vec3(2.0f, 2.0f, 2.0f);
        lightShader->setLightPosition(lightPos);
        lightShader->setCameraPosition(camera->camera->eye);
        lightShader->setMagnitude(1.0f);

        resultingSurfaceTriangle->bindBuffers(lightShader);
        resultingSurfaceTriangle->draw();
    }
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
    ImGui::Begin("CG2");

    bool recalc = false;
    ImGui::Checkbox("Display Grid", &displayGrid);
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Generated GPUMesh", &displaySurface);
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
            ImGui::SameLine();
            recalc |= ImGui::Checkbox("Use Relative K", &useRelativeK);
            if (useRelativeK) {
                recalc |= ImGui::SliderFloat("Relative K", &relativeK, 0.01f, 1.0f);
                radiusSelectionInfo.k = (int)(sourcePoints.size() * relativeK);
                ImGui::LabelText("", "K = %d", radiusSelectionInfo.k);
            } else {
                recalc |= ImGui::SliderInt("Amount", &radiusSelectionInfo.k, 1, 50);
            }
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

    ImGui::Text("Surface rendermode: ");
    recalc |= ImGui::RadioButton("Wireframe", (int*)&surfaceRenderMode, SurfaceRenderMode::WIREFRAME);
    ImGui::SameLine();
    recalc |= ImGui::RadioButton("Solid", (int*)&surfaceRenderMode, SurfaceRenderMode::SOLID);

    if (recalc) {
        updateGeneratedData();
    }

    ImGui::End();
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
    delete gridLinesMesh;
    gridLinesMesh = nullptr;

    delete resultingSurfaceWireframe;
    resultingSurfaceWireframe = nullptr;

    delete resultingSurfaceTriangle;
    resultingSurfaceTriangle = nullptr;

    delete surfaceNormalLines;
    surfaceNormalLines = nullptr;

    delete bezierBaseSurface;
    bezierBaseSurface = nullptr;
}

void ParametricSurfaceViewer::createGrid() {
    gridLinesMesh = generateXYGridLinesMesh(uDivisions, vDivisions, boundingBox);
}

int subdivideToDivisions(int divisions, int subdivisionLevel) {
    return divisions + (divisions - 1) * (int)pow(2, subdivisionLevel - 1);
}

void ParametricSurfaceViewer::createSurfaceAndNormals_MLS() {
    int xDiv = subdivideToDivisions(uDivisions, subdivisionLevel);
    int yDiv = subdivideToDivisions(vDivisions, subdivisionLevel);

    std::vector<glm::vec3> points = calcXYGridPoints(xDiv, yDiv, boundingBox);
    std::vector<glm::vec3> normals(points.size());

    setDataWithMovingLeastSquares(points, normals, kdTree,
        radiusSelectionInfo, leastSquaresSolver, parallelSurfaceGeneration);

    if(SurfaceRenderMode::WIREFRAME == surfaceRenderMode)
        resultingSurfaceWireframe = new WireframeGPUMesh<VertexP>(createVertexPVector(points), calcGridEdges(xDiv, yDiv));
    else
        resultingSurfaceTriangle = new TriangleGPUMesh<VertexPN>(createVertexPNVector(points, normals), calcGridTriangleIndices(xDiv, yDiv));

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
    for (int i = 0; i < stride; i++) {
        std::vector<glm::vec3> controls;
        for (unsigned int j = 0; j < gridPoints.size() / stride; j++) {
            controls.push_back(gridPoints[j * stride + i]);
        }
        curves.push_back(BezierCurve(controls));
    }
    return curves;
}

struct PositionsAndNormals {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
};

PositionsAndNormals calcBezierSurface(std::vector<glm::vec3> &gridPoints, int stride, int uDivisions, int vDivisions) {
    auto curves1 = bezierCurvesFromGrid_Columns(gridPoints, stride);
    auto points1 = gridFromBezierCurves(curves1, uDivisions, vDivisions);

    auto curves2 = bezierCurvesFromGrid_Rows(gridPoints, stride);
    auto points2 = gridFromBezierCurves(curves2, vDivisions, uDivisions);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    for (int u = 0; u < uDivisions; u++) {
        for (int v = 0; v < vDivisions; v++) {
            int index1 = u * vDivisions + v;
            int index2 = v * uDivisions + u;
            positions.push_back(points1[index1].position);
            normals.push_back(glm::normalize(glm::cross(points2[index2].tangent, points1[index1].tangent)));
        }
    }

    PositionsAndNormals result = {positions, normals};
    return result;
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

    auto surfaceData = calcBezierSurface(basePoints, yBaseDiv, xDiv, yDiv);

    if(SurfaceRenderMode::WIREFRAME == surfaceRenderMode)
        resultingSurfaceWireframe = new WireframeGPUMesh<VertexP>(createVertexPVector(surfaceData.positions), calcGridEdges(xDiv, yDiv));
    else
        resultingSurfaceTriangle = new TriangleGPUMesh<VertexPN>(createVertexPNVector(surfaceData.positions, surfaceData.normals), calcGridTriangleIndices(xDiv, yDiv));

    surfaceNormalLines = createLineSegmentsMesh(surfaceData.positions, surfaceData.normals, normalsLength);

    auto baseSurfaceEdges = calcGridEdges(xBaseDiv, yBaseDiv);
    bezierBaseSurface = new WireframeGPUMesh<VertexP>(createVertexPVector(basePoints), baseSurfaceEdges);
}
