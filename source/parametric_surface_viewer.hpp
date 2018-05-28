#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "bounding_box.hpp"
#include "window_controller.hpp"
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
    int uDivisions = 10;
    int vDivisions = 10;
    float baseGridSize = 1.0f;
    BoundingBox<3> boundingBox;

    /* Surface Generation Settings Settings */
    bool parallelSurfaceGeneration = true;
    float weightRadius = 0.1f;
    float normalsLength = 0.1f;
    int subdivisionLevel = 0;
    int subdivisionType = 0;
    LeastSquaresSolver leastSquaresSolver = LeastSquaresSolver::SVD;

    enum FinalSurfaceType {
        MLS,
        Bezier
    };

    FinalSurfaceType finalSurfaceType = FinalSurfaceType::MLS;

    /* Meshes */
    PointCloudMesh<VertexP>* sourcePointsCloud = nullptr;
    WireframeMesh<VertexP>* resultingSurface = nullptr;
    LinesMesh<VertexP>* surfaceNormalLines = nullptr;
    LinesMesh<VertexP>* gridLinesMesh = nullptr;

};