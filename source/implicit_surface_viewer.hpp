#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "mesh.hpp"
#include "camera.hpp"
#include "shaders.hpp"
#include "bounding_box.hpp"
#include "implicit_curve.hpp"
#include "implicit_surface.hpp"
#include "window_controller.hpp"
#include "parametric_surface_utils.hpp"

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
    void drawCurve();
    void drawPointVisualization();
    void updateGeneratedData();
    void createImplicitCurve();
    void createImplicitSurfaceMesh(std::vector<float> &evaluatedValues, BoundingBox<3> &box);
    void createImplicitSurfaceVisualization(std::vector<float> &evaluatedValues, BoundingBox<3> &box);
    ImplicitSurface *getImplicitSurface();
    BoundingBox<3> getBoundingBox();

    CameraController* camera = nullptr;

    TriangleArrayMesh<VertexPN>* surface = nullptr;
    LinesMesh<VertexP>* curve = nullptr;
    PointCloudMesh<VertexPN>* sourcePositionsMesh = nullptr;
    PointCloudMesh<VertexPC>* implicitSurfacePoints = nullptr;

    FlatShader* flatShader = nullptr;
    NormalShader* normalShader = nullptr;
    ShadelessColorShader *shadelessColorShader = nullptr;

    std::vector<glm::vec3> sourcePositions;
    std::vector<glm::vec3> sourceNormals;
    float radius = 1;
    float boundingBoxSize = 2;
    int resolution = 10;
    bool flipInAndOutside = false;

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
};
