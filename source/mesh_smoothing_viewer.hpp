#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "gpu_mesh.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "shaders.hpp"
#include "bounding_box.hpp"
#include "window_controller.hpp"

class MeshSmoothingViewer : public WindowController {
public:
    MeshSmoothingViewer(Window* window)
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
    void drawSourceMesh();
    void updateGPUData();
    void resetManipulatedMesh();

    CameraController* camera = nullptr;
    TriangleGPUMesh<VertexPN>* gpuMesh = nullptr;
    NormalShader* normalShader = nullptr;
    HalfedgeMesh* sourceMesh = nullptr;
    HalfedgeMesh* manipulatedMesh = nullptr;
    std::vector<unsigned int> sourceConnectivity;

    enum InteractionMode {
        Step,
        Realtime
    };

    InteractionMode mode = InteractionMode::Step;

    struct {
        float factor = 0.5;
        int steps = 1;
    } stepSettings;

    struct {
        float factor = 0.5;
        int steps = 1;
    } realtimeSettings;
};
