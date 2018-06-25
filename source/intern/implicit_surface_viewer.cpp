#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../implicit_curve.hpp"



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

class CassiniCurve : public ImplicitCurve {
public:
    float a, c;

    CassiniCurve(float a = 1, float c = 1)
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



bool ImplicitSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
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
    bool recalc = false;
    ImGui::Begin("cg2");
    ImGui::Text("Close App by ESC");
    ImGui::Text("Zoom with mouse wheel");
    ImGui::Text("Rotate by holding left Ctrl + left mouse button");
    recalc |= ImGui::SliderFloat("Radius", &radius, 0.0, 2.0);
    recalc |= ImGui::SliderFloat("Bounding Box Size", &boundingBoxSize, 0.0, 10.0);
    recalc |= ImGui::SliderInt("Resolution", &resolution, 5, 200);

    recalc |= ImGui::SliderFloat("a", &a, 0.1f, 2.0f);
    recalc |= ImGui::SliderFloat("c", &c, 0.1f, 2.0f);

    if (recalc) {
        updateGeneratedData();
    }

    ImGui::End();
}

void ImplicitSurfaceViewer::updateGeneratedData() {
    delete curve;
    curve = nullptr;

    BoundingBox<2> box;
    box.min[0] = -boundingBoxSize;
    box.min[1] = -boundingBoxSize;
    box.max[0] =  boundingBoxSize;
    box.max[1] =  boundingBoxSize;

    CassiniCurve cassiniCurve = CassiniCurve(a, c);
    curve = linesFromImplicitCurve(cassiniCurve, box, resolution);
}
