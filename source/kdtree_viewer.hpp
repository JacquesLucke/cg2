#pragma once

#include <glm/glm.hpp>
#include <cmath>

#include "window_controller.hpp"
#include "gpu_mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "kdtree.hpp"
#include "ray.hpp"
#include "shaders.hpp"

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

    TriangleGPUMesh<VertexP> *getConsideredBoxesMesh();
    std::vector<KDTreeVec3::BoundingBoxWithDepth> getConsideredBoxes();
    void resetQueryResults();

    void runKDTreePerformanceTest();

    enum CollectMode {
        RADIUS, KNEAREST
    };

    GPUMesh<VertexPN>* mesh = nullptr;
    FlatShader* flatShader = nullptr;
    NormalShader* normalShader = nullptr;
    CameraController* camera = nullptr;

    KDTreeVec3 *kdTree = nullptr;
    std::vector<glm::vec3> kdTreePoints;

    glm::vec3 queryCenter = glm::vec3(0, 0, 0);
    CollectMode collectMode = RADIUS;
    int meshDrawMode = GL_LINE;

    float collectRadius = 1;
    int collectAmount = 5;

    int boxDepth = 4;

    bool shouldDrawBoxes = false;
    float meshBrightness = 0.5f;

    TriangleGPUMesh<VertexP> *consideredBoxesMesh = nullptr;
    PointCloudGPUMesh<VertexP> *collectedPoints = nullptr;
};
