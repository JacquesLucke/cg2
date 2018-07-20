#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "../camera.hpp"
#include "../kdtree_viewer.hpp"
#include "../resources.hpp"
#include "../bounding_box.hpp"
#include "../mesh_utils.hpp"
#include "../random.hpp"
#include "../timer.hpp"

bool KDTreeViewer::onSetup() {
    OffFileData *offData = loadRelOffResource("teapot.off");
    assert(offData != nullptr);

    mesh = offDataToTriangleMesh_VertexPN(offData);

    kdTreePoints = std::vector<glm::vec3>(offData->positions.begin(), offData->positions.end());
    kdTree = new KDTreeVec3(kdTreePoints.data(), kdTreePoints.size(), 5);
    kdTree->balance();

    delete offData;

    flatShader = new FlatShader();
    normalShader = new NormalShader();

    //runKDTreePerformanceTest();

    return true;
}

void KDTreeViewer::runKDTreePerformanceTest() {
    for (int i = 12; i < 25; i++) {
        unsigned int amount = (unsigned int)pow(2, i);
        std::cout << "Amount: " << amount << std::endl;
        std::vector<glm::vec3> points = generateRandomPoints(amount, 0);
        KDTreeVec3 tree(points.data(), points.size(), 5);
        tree.balance();

        for (int j = 0; j < 5; j++){
            TIMEIT("find 1000 nearest")
            glm::vec3 point = glm::vec3((float)j/5.0, 0, 0);
            tree.collectKNearest(point, 1000);
        }
    }
}

void KDTreeViewer::onUpdate() {
    if (!ImGui::GetIO().WantCaptureMouse) {
        camera->update(getElapsedMilliseconds());
        performSelection();
    }
}

void KDTreeViewer::onRender() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();

    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    normalShader->bind();
    normalShader->setViewProj(matViewProj);
    normalShader->resetModelMatrix();
    flatShader->bind();
    flatShader->setViewProj(matViewProj);
    flatShader->resetModelMatrix();

    drawMesh();

    drawQueryPoint();
    drawCollectedPoints();
    if (shouldDrawBoxes && collectMode == RADIUS) drawConsideredBoxes();

    drawPreSelectionPoint();

}

void KDTreeViewer::drawMesh() {
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, meshDrawMode);
    glPointSize(2);
    normalShader->bind();
    normalShader->setBrightness(meshBrightness);
    mesh->bindBuffers(normalShader);
    mesh->draw();
    glDisable(GL_DEPTH_TEST);
}

void KDTreeViewer::drawQueryPoint() {
    std::vector<VertexP> vertices;
    vertices.push_back(VertexP(queryCenter));
    PointCloudGPUMesh<VertexP> cloud(vertices);

    glPointSize(10);
    flatShader->bind();
    flatShader->setColor(0, 0, 1);
    cloud.bindBuffers(flatShader);
    cloud.draw();
}

void KDTreeViewer::drawPreSelectionPoint() {
    glm::vec3 point = getPreSelectedPoint();

    std::vector<VertexP> vertices;
    vertices.push_back(VertexP(point));
    PointCloudGPUMesh<VertexP> cloud(vertices);

    glPointSize(5);
    flatShader->bind();
    flatShader->setColor(0.3f, 0.3f, 1);
    cloud.bindBuffers(flatShader);
    cloud.draw();
}

void KDTreeViewer::drawCollectedPoints() {
    if (collectedPoints == nullptr) {
        std::vector<glm::vec3> points = getCollectedPoints();
        collectedPoints = new PointCloudGPUMesh<VertexP>(createVertexPVector(points));
    }

    glPointSize(5);
    flatShader->bind();
    flatShader->setColor(1, 0, 0);
    collectedPoints->bindBuffers(flatShader);
    collectedPoints->draw();
}

void KDTreeViewer::drawConsideredBoxes() {
    flatShader->bind();
    flatShader->setColor(0, 1, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (consideredBoxesMesh == nullptr) {
        consideredBoxesMesh = getConsideredBoxesMesh();
    }

    consideredBoxesMesh->bindBuffers(flatShader);
    consideredBoxesMesh->draw();
}

TriangleGPUMesh<VertexP> *KDTreeViewer::getConsideredBoxesMesh() {
    auto boxes = getConsideredBoxes();

    std::vector<VertexP> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < boxes.size(); i++) {
        if (boxes[i].depth == boxDepth) {
            appendBoxTriangles(vertices, indices, boxes[i].box);
        }
    }

    return new TriangleGPUMesh<VertexP>(vertices, indices);
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

void KDTreeViewer::onRenderUI() {
    ImGui::Begin("CG2");

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

    ImGui::SliderFloat("Mesh Brightness", &meshBrightness, 0.0, 1.0);
    ImGui::RadioButton("Points", &meshDrawMode, GL_POINT); ImGui::SameLine();
    ImGui::RadioButton("Lines", &meshDrawMode, GL_LINE); ImGui::SameLine();
    ImGui::RadioButton("Fill", &meshDrawMode, GL_FILL);

    if (queryChanged) {
        resetQueryResults();
    }

    ImGui::End();
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
