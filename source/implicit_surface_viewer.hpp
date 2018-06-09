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

    CameraController* camera = nullptr;
    TriangleMesh<VertexP>* surface = nullptr;
    FlatShader* flatShader = nullptr;
    float radius = 1;
};