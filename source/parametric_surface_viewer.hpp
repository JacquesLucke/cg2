#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "window_controller.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "parametric_surface_utils.hpp"


class ParametricSurfaceViewer : public WindowController {
public:
    ParametricSurfaceViewer(Window* window)
        : WindowController(window),
          camera(new CameraController(new PerspectiveCamera(
              glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), 1, window->aspect()), window)) {}

protected:
    bool onSetup() final override;
    void onUpdate() final override;
    void onRender() final override;
    void onRenderUI() final override;

private:
    void prepareDrawDimensions();
    void setViewProjMatrixInShaders();
    void drawGrid();
    void drawSourcePoints();
    void drawResultingSurface();
    void resetGeneratedData();


    CameraController* camera;

    int xDivisions = 10;
    int zDivisions = 10;
    float baseGridSize = 1.0f;
    float weightRadius = 0.1f;
    bool displayGeneratedMesh = true;
    bool displaySourcePoints = true;
    bool parallelSurfaceGeneration = true;
    int sourcePointSize = 1;

    LinesMesh<VertexP> *gridLinesMesh = nullptr;
    std::vector<glm::vec3>* gridPoints = nullptr;
    FlatShader* flatShader = nullptr;

    std::vector<glm::vec3> sourcePoints;
    PointCloudMesh<VertexP>* sourcePointsCloud = nullptr;
    WireframeMesh<VertexP>* resultingSurface = nullptr;

    KDTreeVec3_2D* kdTree = nullptr;
};