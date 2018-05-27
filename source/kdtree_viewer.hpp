#pragma once

#include <glm/glm.hpp>
#include <cmath>

#include "window_controller.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "kdtree.hpp"
#include "ray.hpp"
#include "shaders.hpp"

inline float getVec3DistanceSquared(glm::vec3 &a, glm::vec3 &b) {
    float xDiff = a.x - b.x;
    float yDiff = a.y - b.y;
    float zDiff = a.z - b.z;
    return xDiff * xDiff + yDiff * yDiff + zDiff * zDiff;
}

inline float getVec3Distance(glm::vec3 &a, glm::vec3 &b) {
    return sqrt(getVec3DistanceSquared(a, b));
}

using KDTreeVec3 = KDTree<glm::vec3, 3, getVec3Distance>;

class KDTreeViewer : public WindowController {
public:
    KDTreeViewer(Window* window)
        : WindowController(window),
          camera(new CameraController(new PerspectiveCamera(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), 1, window->aspect()), window)) {}

protected:
    bool onSetup() final override;
    //void onTeardown() final override;
    void onUpdate() final override;
    void onRender() final override;
    void onRenderUI() final override;

private:
    Ray getMouseRay();

    void drawMesh();
    void drawQueryPoint();
    void drawPreSelectionPoint();
    void drawCollectedPoints();
    void drawConsideredBoxes();

    void performSelection();
    glm::vec3 getPreSelectedPoint();
    std::vector<glm::vec3> getCollectedPoints();

    TriangleMesh<VertexP> *getConsideredBoxesMesh();
    std::vector<KDTreeVec3::BoundingBoxWithDepth> getConsideredBoxes();
    void resetQueryResults();

    void runKDTreePerformanceTest();

    enum CollectMode {
        RADIUS, KNEAREST
    };

    Mesh<VertexPN>* mesh;
    FlatShader* flatShader;
    SolidShader* solidShader;
    CameraController* camera;

    KDTreeVec3 *kdTree;
    std::vector<glm::vec3> kdTreePoints;

    glm::vec3 queryCenter = glm::vec3(0, 0, 0);
    CollectMode collectMode = RADIUS;
    int meshDrawMode = GL_LINE;

    float collectRadius = 1;
    int collectAmount = 5;

    int boxDepth = 4;

    bool shouldDrawBoxes = false;
    float meshBrightness = 0.5f;

    TriangleMesh<VertexP> *consideredBoxesMesh = nullptr;
    PointCloudMesh<VertexP> *collectedPoints = nullptr;
};
