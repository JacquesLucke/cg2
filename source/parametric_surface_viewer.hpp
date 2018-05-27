#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "window_controller.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class ParametricSurfaceViewer : public WindowController {
public:
    ParametricSurfaceViewer(Window* window)
        : WindowController(window),
          camera(new CameraController(new PerspectiveCamera(
              glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), 1, window->aspect()), window)) {}

protected:
    bool onSetup() final override;
    void onRender() final override;
    void onRenderUI() final override;

private:
    CameraController* camera;

    int xDivisions = 10;
    int zDivisions = 10;

    std::vector<glm::vec3>* gridPoints = nullptr;
    FlatShader* gridShader = nullptr;
};