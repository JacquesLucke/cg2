#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "mesh.hpp"
#include "camera.hpp"
#include "shaders.hpp"
#include "bounding_box.hpp"
#include "window_controller.hpp"
#include "parametric_surface_utils.hpp"


class ParametricSurfaceViewer : public WindowController {
public:
    ParametricSurfaceViewer(Window* window)
        : WindowController(window),
          camera(new CameraController(new PerspectiveCamera(
              glm::vec3(2, 1, 2), glm::vec3(0, 0, 0), 1, window->aspect()), window)) {}

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
    void drawBezierBase();

    void updateGeneratedData();
    void deleteGeneratedData();
    void createGrid();
    void createSurfaceAndNormals_MLS();
    void createSurfaceAndNormals_Bezier();


    CameraController* camera;
    KDTreeVec3_2D* kdTree = nullptr;
    FlatShader* flatShader = nullptr;
    ShadelessColorShader* colorShader = nullptr;
    std::vector<glm::vec3> sourcePoints;

    /* Display Settings */
    bool displayGrid = true;
    bool displaySurface = true;
    bool displaySourcePoints = true;
    bool displayNormals = false;
    bool displayBezierBase = false;
    int sourcePointSize = 1;
    bool useDepthTest = false;

    /* Base Grid Settings */
    int uDivisions = 10;
    int vDivisions = 10;
    float baseGridSize = 1.0f;
    BoundingBox<3> boundingBox;

    /* Surface Generation Settings Settings */
    LeastSquaresSolver leastSquaresSolver = LeastSquaresSolver::SVD;
    bool parallelSurfaceGeneration = true;
    float normalsLength = 0.1f;
    int subdivisionLevel = 0;
    int subdivisionType = 0;
    RadiusSelectionInfo radiusSelectionInfo;
    bool useRelativeK = false;
    float relativeK = 0.1f;

    enum FinalSurfaceType {
        MLS,
        Bezier
    };

    FinalSurfaceType finalSurfaceType = FinalSurfaceType::MLS;

    /* Meshes */
    LinesMesh<VertexP>* gridLinesMesh = nullptr;
    PointCloudMesh<VertexPC>* sourcePointsCloud = nullptr;
    WireframeMesh<VertexP>* resultingSurface = nullptr;
    LinesMesh<VertexP>* surfaceNormalLines = nullptr;
    WireframeMesh<VertexP>* bezierBaseSurface = nullptr;
};
