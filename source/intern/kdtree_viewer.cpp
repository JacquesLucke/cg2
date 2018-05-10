#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../camera.hpp"
#include "../kdtree_viewer.hpp"
#include "../resources.hpp"
#include "../bounding_box.hpp"
#include "../mesh_utils.hpp"

bool KDTreeViewer::onSetup() {
    OffFileData *offData = loadRelOffResource("teapot.off");
    assert(offData != nullptr);

    mesh = offDataToTriangleMesh_VertexPN(offData);

    kdTreePoints = std::vector<glm::vec3>(offData->positions.begin(), offData->positions.end());
    kdTree = new KDTreeVec3(kdTreePoints.data(), kdTreePoints.size(), 5);
    kdTree->balance();

    delete offData;

    flatShader = new FlatShader();
    solidShader = new SolidShader();

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

    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    solidShader->bind();
    solidShader->setTransforms(matViewProj);
    flatShader->bind();
    flatShader->setTransforms(matViewProj);

    drawMesh();

    drawQueryPoint();
    drawCollectedPoints();
    if (shouldDrawBoxes && collectMode == RADIUS) drawConsideredBoxes();

    if (!camera->isFlying() && !ImGui::GetIO().WantCaptureMouse) {
        drawPreSelectionPoint();
    }
}

void KDTreeViewer::drawMesh() {
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, meshDrawMode);
    glPointSize(2);
    solidShader->bind();
    solidShader->setBrightness(meshBrightness);
    mesh->draw(solidShader);
    glDisable(GL_DEPTH_TEST);
}

void KDTreeViewer::drawQueryPoint() {
    std::vector<VertexP> vertices;
    vertices.push_back(VertexP(queryCenter));
    PointCloud<VertexP> cloud(vertices);

    glPointSize(10);
    flatShader->bind();
    flatShader->setColor(0, 0, 1);
    cloud.draw(flatShader);
}

void KDTreeViewer::drawPreSelectionPoint() {
    glm::vec3 point = getPreSelectedPoint();

    std::vector<VertexP> vertices;
    vertices.push_back(VertexP(point));
    PointCloud<VertexP> cloud(vertices);

    glPointSize(5);
    flatShader->bind();
    flatShader->setColor(0.3f, 0.3f, 1);
    cloud.draw(flatShader);
}

void KDTreeViewer::drawCollectedPoints() {
    if (collectedPoints == nullptr) {
        std::vector<glm::vec3> points = getCollectedPoints();
        collectedPoints = new PointCloud<VertexP>(createVertexPVector(points));
    }

    glPointSize(5);
    flatShader->bind();
    flatShader->setColor(1, 0, 0);
    collectedPoints->draw(flatShader);
}

void KDTreeViewer::drawConsideredBoxes() {
    flatShader->bind();
    flatShader->setColor(0, 1, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (consideredBoxesMesh == nullptr) {
        consideredBoxesMesh = getConsideredBoxesMesh();
    }
    consideredBoxesMesh->draw(flatShader);
}

TriangleMesh<VertexP> *KDTreeViewer::getConsideredBoxesMesh() {
    auto boxes = getConsideredBoxes();

    std::vector<VertexP> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < boxes.size(); i++) {
        if (boxes[i].depth == boxDepth) {
            appendBoxTriangles(vertices, indices, boxes[i].box);
        }
    }

    return new TriangleMesh<VertexP>(vertices, indices);
}

std::vector<KDTreeVec3::BoundingBoxWithDepth> KDTreeViewer::getConsideredBoxes() {
    return kdTree->getBoundingBoxes_Radius(queryCenter, collectRadius);
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
        glm::vec3 oldQueryCenter = queryCenter;
        queryCenter = getPreSelectedPoint();

        if (queryCenter != oldQueryCenter) {
            resetQueryResults();
        }
    }
}

static void drawFlyCameraControls() {
    ImGui::LabelText("", "ESC: stop fly mode");
    ImGui::LabelText("", "WASDQE : move camera");
    ImGui::LabelText("", "mouse: rotate camera");
    ImGui::LabelText("", "scroll wheel: change speed");
}

void KDTreeViewer::onRenderUI() {
    if (camera->isFlying()) {
        drawFlyCameraControls();
        return;
    }

    bool queryChanged = false;

    queryChanged |= ImGui::RadioButton("Radius", (int*)&collectMode, RADIUS); ImGui::SameLine();
    queryChanged |= ImGui::RadioButton("K Nearest", (int*)&collectMode, KNEAREST);

    if (collectMode == RADIUS) {
        queryChanged |= ImGui::SliderFloat("Radius", &collectRadius, 0.0f, 5.0f);
        ImGui::Checkbox("Draw Considered Boxes", &shouldDrawBoxes);
        if (shouldDrawBoxes){
            queryChanged |= ImGui::SliderInt("Depth", &boxDepth, 0, 20);
        }
    } else if (collectMode == KNEAREST) {
        queryChanged |= ImGui::SliderInt("Amount", &collectAmount, 0, 1000);
    }

    ImGui::Separator();

    ImGui::Text("Press F to start fly mode.");
    queryChanged |= ImGui::SliderFloat3("Query Center", (float*)&queryCenter, -10.0f, 10.0f);

    ImGui::SliderFloat("Mesh Brightness", &meshBrightness, 0.0, 1.0);
    ImGui::RadioButton("Points", &meshDrawMode, GL_POINT); ImGui::SameLine();
    ImGui::RadioButton("Lines", &meshDrawMode, GL_LINE); ImGui::SameLine();
    ImGui::RadioButton("Fill", &meshDrawMode, GL_FILL);

    if (queryChanged) {
        resetQueryResults();
    }
}

void KDTreeViewer::resetQueryResults() {
    if (consideredBoxesMesh != nullptr) {
        delete consideredBoxesMesh;
        consideredBoxesMesh = nullptr;
    }
    if (collectedPoints != nullptr) {
        delete collectedPoints;
        collectedPoints = nullptr;
    }
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