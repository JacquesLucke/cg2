#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Eigen/Dense>

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
    void drawNormalLines();
    void updateGPUData();
    void resetManipulatedMesh();
    void loadSourceMesh(std::string name);

    CameraController* camera = nullptr;
    TriangleGPUMesh<VertexPN>* gpuMesh = nullptr;
    LinesGPUMesh<VertexP>* normalLines = nullptr;
    NormalShader* normalShader = nullptr;
    FlatShader* flatShader = nullptr;
    HalfedgeMesh* sourceMesh = nullptr;
    HalfedgeMesh* manipulatedMesh = nullptr;
    std::vector<unsigned int> sourceConnectivity;

    enum InteractionMode {
        Step,
        Realtime,
        Spectral
    };

    enum LaplacianType {
        Uniform,
        Cotan
    };

    enum LaplacianStepType {
        Explicit,
        Implicit
    };

    InteractionMode mode = InteractionMode::Step;
    LaplacianType laplacianType = LaplacianType::Uniform;
    LaplacianStepType laplacianStepType = LaplacianStepType::Explicit;

    struct {
        float factor = 0.5;
        int steps = 1;
    } stepSettings;

    struct {
        float factor = 0.5;
        int steps = 1;
    } realtimeSettings;

    struct {
        std::vector<Eigen::VectorXf> eigenvectors;
        int k = 100;
    } spectrumSettings;

    bool displayAsWireframe = false;
    bool displayNormals = false;
    float normalsLength = 0.1f;
};
