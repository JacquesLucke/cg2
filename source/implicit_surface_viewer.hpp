#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "mesh.hpp"
#include "camera.hpp"
#include "shaders.hpp"
#include "bounding_box.hpp"
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
    void drawCurve();
    void updateGeneratedData();
    void createImplicitCurve();
    void createImplicitSurface();

    CameraController* camera = nullptr;
    TriangleMesh<VertexPN>* surface = nullptr;
    LinesMesh<VertexP>* curve = nullptr;
    FlatShader* flatShader = nullptr;
    NormalShader* normalShader = nullptr;
    float radius = 1;
    float boundingBoxSize = 2;
    int resolution = 10;

    float a = 1, c = 1;
};
