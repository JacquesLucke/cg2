#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../mesh_smoothing_viewer.hpp"
#include "../mesh_smoothing_utils.hpp"
#include "../mesh_utils.hpp"
#include "../timer.hpp"
#include "../utils.hpp"


/* Start of actual Window Controller
********************************************/

bool MeshSmoothingViewer::onSetup() {
    OffFileData* data = loadRelOffResource("suzanne_noisy.off");
    sourceMesh = HalfedgeMesh::fromTriangles(data->positions, data->indices);
    sourceConnectivity = data->indices;
    manipulatedMesh = sourceMesh->copy();

    normalShader = new NormalShader();
    flatShader = new FlatShader();
    updateGPUData();

    return true;
}

void MeshSmoothingViewer::onUpdate() {
    if (isKeyDown(GLFW_KEY_ESCAPE)) {
        exit(0);
    }
    if (!ImGui::GetIO().WantCaptureMouse) {
        camera->update(getElapsedMilliseconds());
    }
}

void MeshSmoothingViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();

    glEnable(GL_DEPTH_TEST);
    drawSourceMesh();
    if (displayNormals) drawNormalLines();
    glDisable(GL_DEPTH_TEST);
}

void MeshSmoothingViewer::prepareDrawDimensions() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();
}

void MeshSmoothingViewer::setViewProjMatrixInShaders() {
    auto matrix = camera->camera->getViewProjectionMatrix();
    normalShader->bind();
    normalShader->setViewProj(matrix);
    flatShader->bind();
    flatShader->setViewProj(matrix);
}

void MeshSmoothingViewer::drawSourceMesh() {
    if (displayAsWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    gpuMesh->bindBuffers(normalShader);
    gpuMesh->draw();
    if (displayAsWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void MeshSmoothingViewer::drawNormalLines() {
    flatShader->bind();
    flatShader->setColor(0.5f, 0.5f, 0.5f);
    flatShader->resetModelMatrix();
    normalLines->bindBuffers(flatShader);
    normalLines->draw();
}

void MeshSmoothingViewer::onRenderUI() {
    ImGui::Begin("CG2");

    bool recalc = false;

    ImGui::Text("Interaction Mode");
    recalc |= ImGui::RadioButton("Step", (int*)&mode, InteractionMode::Step); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Realtime", (int*)&mode, InteractionMode::Realtime);

    ImGui::Separator();

    if (mode == InteractionMode::Step) {

        ImGui::RadioButton("Uniform", (int*)&laplacianType, LaplacianType::Uniform); ImGui::SameLine();
        ImGui::RadioButton("Cotan", (int*)&laplacianType, LaplacianType::Cotan);

        if (laplacianType == LaplacianType::Cotan) {
            ImGui::RadioButton("Explicit", (int*)&laplacianStepType, LaplacianStepType::Explicit); ImGui::SameLine();
            ImGui::RadioButton("Implicit", (int*)&laplacianStepType, LaplacianStepType::Implicit);
        }

        if (ImGui::Button("Smooth Step")) {
            if (laplacianType == LaplacianType::Uniform) {
                smooth_UniformLaplacian(*manipulatedMesh, stepSettings.factor, stepSettings.steps);
            } else {
                bool doImplicitStep = laplacianStepType == LaplacianStepType::Implicit;
                smooth_CotanLaplacian(*manipulatedMesh, stepSettings.factor, stepSettings.steps, doImplicitStep);
            }
            recalc |= true;
        }
        if (ImGui::Button("Reset")) {
            resetManipulatedMesh();
            recalc |= true;
        }
        ImGui::SliderFloat("Factor", &stepSettings.factor, 0, 1);
        ImGui::SliderInt("Steps", &stepSettings.steps, 1, 200);
    } else if (mode == InteractionMode::Realtime) {
        recalc |= ImGui::SliderFloat("Factor", &realtimeSettings.factor, 0.0, 1.0);
        recalc |= ImGui::SliderInt("Steps", &realtimeSettings.steps, 0, 20);

        if (recalc) {
            resetManipulatedMesh();
            smooth_UniformLaplacian(*manipulatedMesh, realtimeSettings.factor, realtimeSettings.steps);
        }
    }

    ImGui::Separator();
    ImGui::Checkbox("Draw Wireframe", &displayAsWireframe);
    ImGui::Checkbox("Draw Normals", &displayNormals);
    if (displayNormals) {
        recalc |= ImGui::SliderFloat("Normals Length", &normalsLength, 0.0f, 0.5f);
    }

    if (recalc) {
        updateGPUData();
    }

    ImGui::End();
}

void MeshSmoothingViewer::resetManipulatedMesh() {
    manipulatedMesh = sourceMesh->copy();
}

void MeshSmoothingViewer::updateGPUData() {
    TIMEIT("update gpu data");
    delete gpuMesh;
    delete normalLines;

    auto positions = manipulatedMesh->getVertexPositions();
    auto normals = calculateVertexNormals(positions, sourceConnectivity);
    auto vertices = createVertexPNVector(positions, normals);
    gpuMesh = new TriangleGPUMesh<VertexPN>(vertices, sourceConnectivity);
    normalLines = createLineSegmentsMesh(positions, normals, normalsLength);
}