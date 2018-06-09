#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"

typedef float (*ImplicitSurfaceFunction)(glm::vec3 &position);

class ImplicitCurve {
public:
    virtual float evaluate(glm::vec2 &position) = 0;
};

class ImplicitCircleCurve : public ImplicitCurve {
public:
    float radius;

    ImplicitCircleCurve(float radius = 1)
        : radius(radius) {}

    float evaluate(glm::vec2 &position) final override {
        return glm::length(position) - radius;
    }
};

class ImplicitSinCosCurve : public ImplicitCurve {
    float evaluate(glm::vec2 &position) final override {
        float x = position.x;
        float y = position.y;
        return std::sin(x + y) - std::cos(x * y) + 1;
    }
};

TriangleMesh<VertexP> *meshImplicitSurface(ImplicitSurfaceFunction f) {
    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;

    positions.push_back(glm::vec3(0, 0, 0));
    positions.push_back(glm::vec3(1, 0, 0));
    positions.push_back(glm::vec3(0, 1, 0));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    return new TriangleMesh<VertexP>(createVertexPVector(positions), indices);
}

char lineTable[16][2] = {
    {-1, -1},
    { 3, -1},
    { 0, -1},
    { 5, -1},
    { 1, -1},
    { 1,  3},
    { 4, -1},
    { 2, -1},

    { 2, -1},
    { 4, -1},
    { 0,  2},
    { 1, -1},
    { 5, -1},
    { 0, -1},
    { 3, -1},
    {-1, -1}
};

char edgeTable[6][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    {0, 2},
    {1, 3}
};

float center(float a, float b) {
    return (a + b) / 2.0f;
}

LinesMesh<VertexP> *linesFromImplicitCurve(ImplicitCurve &curve, BoundingBox<2> box, int resolution) {
    std::vector<glm::vec3> positions;

    for (int x = 0; x < resolution; x++) {
        for (int y = 0; y < resolution; y++) {
            float x0 = box.mapToBox(x / (float)resolution, 0);
            float x1 = box.mapToBox((x+1) / (float)resolution, 0);
            float y0 = box.mapToBox(y / (float)resolution, 1);
            float y1 = box.mapToBox((y+1) / (float)resolution, 1);

            float centerX = center(x0, x1);
            float centerY = center(y0, y1);

            glm::vec2 edgePoints[4] = {
                glm::vec2(centerX, y0),
                glm::vec2(x1, centerY),
                glm::vec2(centerX, y1),
                glm::vec2(x0, centerY)
            };

            char lookupIndex = (
                ((curve.evaluate(glm::vec2(x0, y0)) < 0) << 0) |
                ((curve.evaluate(glm::vec2(x1, y0)) < 0) << 1) |
                ((curve.evaluate(glm::vec2(x1, y1)) < 0) << 2) |
                ((curve.evaluate(glm::vec2(x0, y1)) < 0) << 3)
            );

            for (int i = 0; i < 2; i++) {
                char edge = lineTable[lookupIndex][i];
                if (edge == -1) break;

                for (int j = 0; j < 2; j++){
                    positions.push_back(glm::vec3(edgePoints[edgeTable[edge][j]], 0));
                }
            }
        }
    }

    return new LinesMesh<VertexP>(createVertexPVector(positions));
}

float implicitSurface_Sphere(glm::vec3 &position) {
    return glm::length(position) - 1;
}

bool ImplicitSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    surface = meshImplicitSurface(implicitSurface_Sphere);
    updateGeneratedData();
    return true;
}

void ImplicitSurfaceViewer::onUpdate() {
    camera->update(GLFW_KEY_F, getElapsedMilliseconds());
}

void ImplicitSurfaceViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();
    // drawSurface();
    drawCurve();
}

void ImplicitSurfaceViewer::prepareDrawDimensions() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();
}

void ImplicitSurfaceViewer::setViewProjMatrixInShaders() {
    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    flatShader->bind();
    flatShader->setViewProj(matViewProj);
}

void ImplicitSurfaceViewer::drawSurface() {
    flatShader->bind();
    flatShader->resetModelMatrix();
    flatShader->setColor(1, 0, 0);
    surface->bindBuffers(flatShader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    surface->draw();
}

void ImplicitSurfaceViewer::drawCurve() {
    flatShader->bind();
    flatShader->resetModelMatrix();
    flatShader->setColor(1, 1, 0);
    curve->bindBuffers(flatShader);
    curve->draw();
}

void ImplicitSurfaceViewer::onRenderUI() {
    if (camera->isFlying()) {
        ImGui::Text("Disable fly mode with ESC");
        return;
    }
    bool recalc = false;
    recalc |= ImGui::SliderFloat("Radius", &radius, 0.0, 2.0);
    recalc |= ImGui::SliderFloat("Bounding Box Size", &boundingBoxSize, 0.0, 10.0);
    recalc |= ImGui::SliderInt("Resolution", &resolution, 5, 200);

    if (recalc) {
        updateGeneratedData();
    }
}

void ImplicitSurfaceViewer::updateGeneratedData() {
    delete curve;
    curve = nullptr;

    BoundingBox<2> box;
    box.min[0] = -boundingBoxSize;
    box.min[1] = -boundingBoxSize;
    box.max[0] =  boundingBoxSize;
    box.max[1] =  boundingBoxSize;

    curve = linesFromImplicitCurve(ImplicitSinCosCurve(), box, resolution);
}