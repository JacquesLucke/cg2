#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../mesh_smoothing_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../timer.hpp"
#include "../utils.hpp"


/* Start of actual Window Controller
********************************************/

bool MeshSmoothingViewer::onSetup() {
    OffFileData* data = loadRelOffResource("suzanne_noisy.off");
    std::vector<glm::vec3> normals = calculateVertexNormals(data->positions, data->indices);

    auto vertices = createVertexPNVector(data->positions, normals);
    sourceGPUMesh = new TriangleGPUMesh<VertexPN>(vertices, data->indices);
    sourceMesh = HalfEdgeMesh::fromTriangles(data->positions, data->indices);

    normalShader = new NormalShader();

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
    sourceGPUMesh->bindBuffers(normalShader);
    sourceGPUMesh->draw();
}

void MeshSmoothingViewer::onRenderUI() {
    ImGui::Begin("CG2");
    ImGui::End();
}