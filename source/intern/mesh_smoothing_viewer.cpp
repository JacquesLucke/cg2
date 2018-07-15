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
    sourceMesh = HalfEdgeMesh::fromTriangles(data->positions, data->indices);
    manipulatedMesh = sourceMesh->copy();

    normalShader = new NormalShader();
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
}

void MeshSmoothingViewer::drawSourceMesh() {
    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    gpuMesh->bindBuffers(normalShader);
    gpuMesh->draw();
}

void MeshSmoothingViewer::onRenderUI() {
    ImGui::Begin("CG2");

    bool recalc = false;

    if (ImGui::Button("Smooth Step")) {
        smooth_UniformLaplacian_OneStep(*manipulatedMesh, 0.6f);
        recalc |= true;
    }

    if (ImGui::Button("Reset")) {
        manipulatedMesh = sourceMesh->copy();
        recalc |= true;
    }

    if (recalc) {
        updateGPUData();
    }

    ImGui::End();
}

void MeshSmoothingViewer::updateGPUData() {
    delete gpuMesh;

    std::vector<glm::vec3> positions;
    for (int i = 0; i < manipulatedMesh->getVertexAmount(); i++) {
        positions.push_back(manipulatedMesh->getVertexPosition(i));
    }

    std::vector<unsigned int> indices;
    for (int i = 0; i < manipulatedMesh->getFaceAmount(); i++) {
        auto faceIndices = manipulatedMesh->neighbours_Face_VertexIndices(i);
        assert(faceIndices.size() == 3);
        for (int j = 0; j < 3; j++) {
            indices.push_back(faceIndices[j]);
        }
    }

    auto normals = calculateVertexNormals(positions, indices);
    auto vertices = createVertexPNVector(positions, normals);

    gpuMesh = new TriangleGPUMesh<VertexPN>(vertices, indices);
}