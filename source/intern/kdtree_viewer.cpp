#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../camera.hpp"
#include "../kdtree_viewer.hpp"
#include "../resources.hpp"
#include "../bounding_box.hpp"

std::vector<VertexP> convertVector_Vec3ToVertexP(std::vector<glm::vec3> &vectors) {
    std::vector<VertexP> vertices;
    for (unsigned int i = 0; i < vectors.size(); i++) {
        vertices.push_back(VertexP(vectors[i]));
    }
    return vertices;
}

std::vector<VertexP> extractVerticesFromOffData(OffFileData *offData) {
    return convertVector_Vec3ToVertexP(offData->positions);
}

TriangleMesh<VertexP> *offDataToTriangleMesh(OffFileData *offData) {
    std::vector<VertexP> vertices = extractVerticesFromOffData(offData);
    return new TriangleMesh<VertexP>(vertices, offData->indices);
}

PointCloud<VertexP> *offDataToPointCloud(OffFileData *offData) {
    std::vector<VertexP> vertices = extractVerticesFromOffData(offData);
    return new PointCloud<VertexP>(vertices);
}

void appendBoxTriangles(std::vector<VertexP> &vertices, std::vector<unsigned int> &indices, BoundingBox<3> &box) {
    int indexOffset = vertices.size();

    float *limits = (float*)&box;
    for (unsigned int i = 0; i < 8; i++) {
        int xType = (i / 4) % 2;
        int yType = (i / 2) % 2;
        int zType = (i / 1) % 2;
        glm::vec3 point = glm::vec3(
            limits[xType * 3 + 0],
            limits[yType * 3 + 1],
            limits[zType * 3 + 2]);
        vertices.push_back(VertexP(point));
    }

    unsigned int newIndices[6][4] = {
        {0, 1, 2, 3}, {4, 5, 6, 7},
        {0, 1, 4, 5}, {2, 3, 6, 7},
        {0, 2, 4, 6}, {1, 3, 5, 7}
    };

    for (unsigned int i = 0; i < 6; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            indices.push_back(newIndices[i][j] + indexOffset);
        }
        for (unsigned int j = 1; j < 4; j++) {
            indices.push_back(newIndices[i][j] + indexOffset);
        }
    }
}

bool KDTreeViewer::onSetup() {
    OffFileData *offData = loadRelOffResource("teapot.off");
    assert(offData != nullptr);

    mesh = offDataToTriangleMesh(offData);

    kdTreePoints = std::vector<glm::vec3>(offData->positions.begin(), offData->positions.end());
    kdTree = new KDTreeVec3(kdTreePoints.data(), kdTreePoints.size(), 5);
    kdTree->balance();

    delete offData;

    flatShader = new FlatShader();

    return true;
}

void KDTreeViewer::onUpdate() {
    if (!camera->isFlying() && isKeyDown(GLFW_KEY_F)) {
        camera->enableFlyMode();
    }
    if (camera->isFlying() && isKeyDown(GLFW_KEY_ESCAPE)) {
        camera->disableFlyMode();
    }

    camera->update();

    if (!camera->isFlying() && !ImGui::GetIO().WantCaptureMouse) {
        performSelection();
    }
}

void KDTreeViewer::onRender() {
    int width, height;
    glfwGetFramebufferSize(window()->handle(), &width, &height);
    glViewport(0, 0, width, height);
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();

    flatShader->bind();
    flatShader->setColor(color);
    flatShader->setTransforms(camera->camera->getViewProjectionMatrix());

    mesh->draw(flatShader);

    drawQueryPoint();
    drawCollectedPoints();
    drawConsideredBoxes();

    if (!camera->isFlying() && !ImGui::GetIO().WantCaptureMouse) {
        drawPreSelectionPoint();
    }
}

static void drawFlyCameraControls() {
    ImGui::LabelText("", "ESC: stop fly mode");
    ImGui::LabelText("", "WASDQE : move camera");
    ImGui::LabelText("", "mouse: rotate camera");
    ImGui::LabelText("", "scroll wheel: change speed");
}

void KDTreeViewer::drawQueryPoint() {
    std::vector<VertexP> vertices;
    vertices.push_back(VertexP(queryCenter));
    PointCloud<VertexP> cloud(vertices);

    glPointSize(10);
    flatShader->setColor(0, 0, 1);
    cloud.draw(flatShader);
}

void KDTreeViewer::drawPreSelectionPoint() {
    glm::vec3 point = getPreSelectedPoint();

    std::vector<VertexP> vertices;
    vertices.push_back(VertexP(point));
    PointCloud<VertexP> cloud(vertices);

    glPointSize(5);
    flatShader->setColor(0.3f, 0.3f, 1);
    cloud.draw(flatShader);
}

void KDTreeViewer::drawCollectedPoints() {
    std::vector<glm::vec3> points = getCollectedPoints();
    PointCloud<VertexP> cloud(convertVector_Vec3ToVertexP(points));
    glPointSize(5);
    flatShader->setColor(1, 0, 0);
    cloud.draw(flatShader);
}

void KDTreeViewer::drawConsideredBoxes() {
    flatShader->setColor(0, 1, 1);

    std::vector<VertexP> vertices;
    std::vector<unsigned int> indices;
    std::vector<KDTreeVec3::BoundingBoxWithDepth> boxes = kdTree->getBoundingBoxes_Radius(queryCenter, collectRadius);
    for (unsigned int i = 0; i < boxes.size(); i++) {
        if (boxes[i].depth == boxDepth) {
            appendBoxTriangles(vertices, indices, boxes[i].box);
        }
    }

    TriangleMesh<VertexP> *boxMesh = new TriangleMesh<VertexP>(vertices, indices);
    boxMesh->draw(flatShader);
    delete boxMesh;
}

std::vector<glm::vec3> KDTreeViewer::getCollectedPoints() {
    if (collectMode == RADIUS) {
        return kdTree->collectInRadius(queryCenter, collectRadius);
    } else if (collectMode == KNEAREST) {
        return kdTree->collectKNearest(queryCenter, collectAmount);
    }
    assert(false);
    return std::vector<glm::vec3>();
}

void KDTreeViewer::performSelection() {
    if (glfwGetMouseButton(window()->handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        queryCenter = getPreSelectedPoint();
    }
}

void KDTreeViewer::onRenderUI() {
    if (camera->isFlying()) {
        drawFlyCameraControls();
        return;
    }

    ImGui::RadioButton("Radius", (int*)&collectMode, RADIUS); ImGui::SameLine();
    ImGui::RadioButton("K Nearest", (int*)&collectMode, KNEAREST);

    if (collectMode == RADIUS) {
        ImGui::SliderFloat("Radius", &collectRadius, 0.0f, 5.0f);
    } else if (collectMode == KNEAREST) {
        ImGui::SliderInt("Amount", &collectAmount, 0, 1000);
    }

    ImGui::SliderInt("Depth", &boxDepth, 0, 20);

    ImGui::Separator();

    ImGui::Text("Press F to start fly mode.");
    ImGui::ColorEdit3("Wireframe Color", (float*)&color, ImGuiColorEditFlags_NoInputs);
    ImGui::SliderFloat3("Query Center", (float*)&queryCenter, -10.0f, 10.0f);
}

bool KDTreeViewer::isKeyDown(int key) {
    return glfwGetKey(window()->handle(), key) == GLFW_PRESS;
}

Ray KDTreeViewer::getMouseRay() {
    glm::vec2 mousePos = getMousePos();
    float x = mousePos.x / window()->width() * 2 - 1;
    float y = mousePos.y / window()->height() * 2 - 1;
    return camera->camera->getViewRay(glm::vec2(x, y));
}

glm::vec3 KDTreeViewer::getPreSelectedPoint() {
    Ray ray = getMouseRay();
    return ray.getPointWithSmallestAngle(kdTreePoints);
}