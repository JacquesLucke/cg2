#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "gpu_mesh.hpp"
#include "camera.hpp"
#include "shaders.hpp"
#include "bounding_box.hpp"
#include "implicit_curve.hpp"
#include "implicit_surface.hpp"
#include "window_controller.hpp"
#include "parametric_surface_utils.hpp"

enum PointsToSurfaceMethod {
    LocalDistanceFunctions,
    ConstantMLS,
    LinearMLS,
    QuadraticMLS
};

class ImplicitSurfaceViewer : public WindowController {
public:
    ImplicitSurfaceViewer(Window* window)
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
    void drawSurface();
    void drawSourcePoints();
    void drawPointVisualization();
    void updateGeneratedData();
    void createImplicitSurfaceMesh(std::vector<float> &evaluatedValues);
    void createImplicitSurfaceVisualization(std::vector<float> &evaluatedValues);
    ImplicitSurface *getImplicitSurface();
    void updateLightPosition();
    BoundingBox<3> getBoundingBox();

    CameraController* camera = nullptr;

    TriangleArrayGPUMesh<VertexPN>* surface = nullptr;
    PointCloudGPUMesh<VertexPN>* sourcePositionsMesh = nullptr;
    PointCloudGPUMesh<VertexPC>* implicitSurfacePoints = nullptr;

    FlatShader* flatShader = nullptr;
    NormalShader* normalShader = nullptr;
    ShadelessColorShader* shadelessColorShader = nullptr;
    BlinnPhongShader* phongShader = nullptr;

    std::vector<glm::vec3> sourcePositions;
    std::vector<glm::vec3> sourceNormals;
    float radius = 1;
    float boundingBoxSize = 2;
    int resolution = 20;
    bool flipInAndOutside = false;
    glm::vec3 relativeLightPosition = glm::vec3(2, 2, 2);
    glm::vec3 lightPosition;
    BoundingBox<3> boundingBox;

    enum SurfaceSource {
        Sphere,
        Genus2,
        Blobs,
        Points
    };

    SurfaceSource surfaceSource = SurfaceSource::Sphere;
    bool displayVisualizationPoints = false;
    bool displaySourcePoints = false;
    bool displayGeneratedMesh = true;

    bool displayAsWireframe = false;

    struct {
        float radius = 1.0f;
    } sphereData;

    struct {
        glm::vec3 position1 = glm::vec3(-0.5, 0, 0);
        glm::vec3 position2 = glm::vec3(0.5, 0, 0);
        float radius1 = 0.6f, radius2 = 0.6f;
    } blobData;

    struct {
        int minPointAmount = 10;
        float boundingBoxFactor = 1.1f;
        float relativeRadius = 0.05f;
        PointsToSurfaceMethod method = PointsToSurfaceMethod::LocalDistanceFunctions;
    } pointsData;
};
