#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"

typedef float (*ImplicitSurfaceFunction)(glm::vec3 &position);

class ImplicitCurve {
public:
    virtual float evaluate(glm::vec2 &position) {
        return evaluate(position.x, position.y);
    }

    virtual float evaluate(float x, float y) {
        return evaluate(glm::vec2(x, y));
    }
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
public:
    float evaluate(float x, float y) final override {
        return std::sin(x + y) - std::cos(x * y) + 1;
    }
};

class CassiniCurves : public ImplicitCurve {
public:
    float a, c;

    CassiniCurves(float a = 1, float c = 1)
        : a(a), c(c) {}

    float evaluate(float x, float y) final override {
        float x2 = x * x;
        float y2 = y * y;
        float c2 = c * c;
        float c4 = c2 * c2;
        float a4 = a * a * a * a;

        return (x2 + y2) * (x2 + y2) - 2 * c2 * (x2 - y2) - (a4 - c4);
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

inline bool haveSameSign(float a, float b) {
    return a*b >= 0.0f;
}

float edgeVertexPosition(float pos1, float pos2, float value1, float value2) {
    if (haveSameSign(value1, value2)) {
        return (pos1 + pos2) / 2;
    }

    value1 = std::abs(value1);
    value2 = std::abs(value2);
    return (pos1 * value2 + pos2 * value1) / (value1 + value2);
}

LinesMesh<VertexP> *linesFromImplicitCurve(ImplicitCurve &curve, BoundingBox<2> box, int resolution) {
    std::vector<glm::vec3> positions;
    float fResolution = (float)resolution;

    for (int x = 0; x < resolution; x++) {
        float x0 = box.mapToBox((x + 0) / fResolution, 0);
        float x1 = box.mapToBox((x + 1) / fResolution, 0);

        for (int y = 0; y < resolution; y++) {
            float y0 = box.mapToBox((y + 0) / fResolution, 1);
            float y1 = box.mapToBox((y + 1) / fResolution, 1);

            float gridValues[4] = {
                curve.evaluate(x0, y0),
                curve.evaluate(x1, y0),
                curve.evaluate(x1, y1),
                curve.evaluate(x0, y1)
            };

            glm::vec2 edgePoints[4] = {
                glm::vec2(edgeVertexPosition(x0, x1, gridValues[0], gridValues[1]), y0),
                glm::vec2(x1, edgeVertexPosition(y0, y1, gridValues[1], gridValues[2])),
                glm::vec2(edgeVertexPosition(x0, x1, gridValues[3], gridValues[2]), y1),
                glm::vec2(x0, edgeVertexPosition(y0, y1, gridValues[0], gridValues[3]))
            };

            char lookupIndex = 0;
            for (int i = 0; i < 4; i++) {
                lookupIndex |= (gridValues[i] < 0) << i;
            }

            char *lines = lineTable[lookupIndex];

            for (int i = 0; i < 2; i++) {
                char line = lines[i];
                if (line == -1) break;

                char *edgeIndices = edgeTable[line];
                for (int j = 0; j < 2; j++){
                    positions.push_back(glm::vec3(edgePoints[edgeIndices[j]], 0));
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

    recalc |= ImGui::SliderFloat("a", &a, 0.1f, 2.0f);
    recalc |= ImGui::SliderFloat("c", &c, 0.1f, 2.0f);

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

    curve = linesFromImplicitCurve(CassiniCurves(a, c), box, resolution);
}