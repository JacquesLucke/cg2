#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../implicit_curve.hpp"
#include "../implicit_surface.hpp"



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

class ImplicitSurfaceCut : public ImplicitCurve {
public:
    float zValue;
    ImplicitSurface* surface;

    ImplicitSurfaceCut(ImplicitSurface* surface, float zValue)
        : zValue(zValue), surface(surface) {}

    float evaluate(float x, float y) {
        return surface->evaluate(x, y, zValue);
    }
};

class ImplicitSphere : public ImplicitSurface {
public:
    float radius;

    ImplicitSphere(float radius = 1)
        : radius(radius) {}

    float evaluate(glm::vec3 &position) {
        return glm::length(position) - radius;
    }
};

class ImplicitGenus2Surface : public ImplicitSurface {
public:
    float evaluate(float x, float y, float z) {
        float x2 = x * x;
        float y2 = y * y;
        float z2 = z * z;
        return 2*y*(y2-3*x2)*(1-z2)+(x2+y2)*(x2+y2)-(9*z2-1)*(1-z2);
    }
};


bool ImplicitSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    normalShader = new NormalShader();
    updateGeneratedData();
    return true;
}

void ImplicitSurfaceViewer::onUpdate() {
    if (isKeyDown(GLFW_KEY_ESCAPE)) {
        exit (0);
    }
    if (!ImGui::GetIO().WantCaptureMouse) {
        camera->update(getElapsedMilliseconds());
    }
}

void ImplicitSurfaceViewer::onRender() {
    prepareDrawDimensions();
    setViewProjMatrixInShaders();
    drawSurface();
    //drawCurve();
}

void ImplicitSurfaceViewer::prepareDrawDimensions() {
    window()->fitGLViewportInWindow();
    ((PerspectiveCamera*)camera->camera)->aspect = window()->aspect();
}

void ImplicitSurfaceViewer::setViewProjMatrixInShaders() {
    glm::mat4 matViewProj = camera->camera->getViewProjectionMatrix();
    flatShader->bind();
    flatShader->setViewProj(matViewProj);
    normalShader->bind();
    normalShader->setViewProj(matViewProj);
}

void ImplicitSurfaceViewer::drawSurface() {
    glEnable(GL_DEPTH_TEST);
    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    surface->bindBuffers(normalShader);
    surface->draw();
    glDisable(GL_DEPTH_TEST);
}

void ImplicitSurfaceViewer::drawCurve() {
    flatShader->bind();
    flatShader->resetModelMatrix();
    flatShader->setColor(1, 1, 0);
    curve->bindBuffers(flatShader);
    curve->draw();
}

void ImplicitSurfaceViewer::onRenderUI() {
    ImGui::Begin("CG2");

    bool recalc = false;
    recalc |= ImGui::RadioButton("Sphere", (int*)&surfaceSource, SurfaceSource::Sphere); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Genus 2", (int*)&surfaceSource, SurfaceSource::Genus2);

    recalc |= ImGui::SliderFloat("Bounding Box Size", &boundingBoxSize, 0.0, 10.0);
    recalc |= ImGui::SliderInt("Resolution", &resolution, 5, 200);

    if (recalc) {
        updateGeneratedData();
    }

    ImGui::End();
}

void ImplicitSurfaceViewer::updateGeneratedData() {
    delete curve;
    curve = nullptr;

    delete surface;
    surface = nullptr;

    //createImplicitCurve();
    createImplicitSurface();
}

void ImplicitSurfaceViewer::createImplicitCurve() {
    // BoundingBox<2> box;
    // box.min[0] = -boundingBoxSize;
    // box.min[1] = -boundingBoxSize;
    // box.max[0] =  boundingBoxSize;
    // box.max[1] =  boundingBoxSize;

    // CassiniCurve cassiniCurve = CassiniCurve(a, c);
    // curve = linesFromImplicitCurve(cassiniCurve, box, resolution);
}

void ImplicitSurfaceViewer::createImplicitSurface() {
    BoundingBox<3> box;
    box.min[0] = box.min[1] = box.min[2] = -boundingBoxSize;
    box.max[0] = box.max[1] = box.max[2] =  boundingBoxSize;

    ImplicitSurface *source;

    if (surfaceSource == SurfaceSource::Sphere) {
        source = new ImplicitSphere(1);
    } else if (surfaceSource == SurfaceSource::Genus2) {
        source = new ImplicitGenus2Surface();
    }

    std::vector<glm::vec3> positions = trianglesFromImplicitSurface(
        *source, box, resolution, resolution, resolution);
    delete source;

    std::vector<glm::vec3> normals = calculateTriangleVertexNormals(positions);
    surface = new TriangleArrayMesh<VertexPN>(createVertexPNVector(positions, normals));
}