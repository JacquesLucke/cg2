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
    void drawSurface();
    void drawSurfaceNormals();

    void updateGeneratedData();
    void deleteGeneratedData();
    void createGrid();
    void createSurfaceAndNormals();


    CameraController* camera;
    KDTreeVec3_2D* kdTree = nullptr;
    FlatShader* flatShader = nullptr;
    std::vector<glm::vec3> sourcePoints;

    /* Display Settings */
    bool displaySurface = true;
    bool displaySourcePoints = true;
    bool displayNormals = true;
    int sourcePointSize = 1;

    /* Base Grid Settings */
    int xDivisions = 10;
    int zDivisions = 10;
    float baseGridSize = 1.0f;

    /* Surface Generation Settings Settings */
    bool parallelSurfaceGeneration = true;
    float weightRadius = 0.1f;
    float normalsLength = 0.1f;
    bool useSubdivision = false;
    int subdivisionLevel = 1;
    int subdivisionType = 0;
    LeastSquaresSolver leastSquaresSolver = LeastSquaresSolver::SVD;

    /* Meshes */
    PointCloudMesh<VertexP>* sourcePointsCloud = nullptr;
    WireframeMesh<VertexP>* resultingSurface = nullptr;
    LinesMesh<VertexP>* surfaceNormalLines = nullptr;
    LinesMesh<VertexP>* gridLinesMesh = nullptr;

};