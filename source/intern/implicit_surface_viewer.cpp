#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../timer.hpp"
#include "../utils.hpp"


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
    glm::vec3 position;

    ImplicitSphere(float radius = 1, glm::vec3 position = glm::vec3(0, 0, 0))
        : radius(radius), position(position) {}

    float evaluate(glm::vec3 &position) {
        return glm::length(position - this->position) - radius;
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

class ImplicitUnionSurface : public ImplicitSurface {
    std::vector<ImplicitSurface*> surfaces;

public:
    ~ImplicitUnionSurface() {
        for (auto surface : surfaces) {
            delete surface;
        }
    }

    void addSurface(ImplicitSurface* surface) {
        surfaces.push_back(surface);
    }

    float evaluate(glm::vec3 &position) {
        if (surfaces.size() == 0) return 1.0f;
        float finalValue = surfaces[0]->evaluate(position);
        for (unsigned int i = 1; i < surfaces.size(); i++) {
            float value = surfaces[i]->evaluate(position);
            if (value < finalValue) finalValue = value;
        }
        return finalValue;
    }
};

class ImplicitSurfaceFromPoints : public ImplicitSurface {

    struct KDTreeEntry {
        glm::vec3 position;
        glm::vec3 normal;

        KDTreeEntry(glm::vec3 position = glm::vec3(0), glm::vec3 normal = glm::vec3(0))
            : position(position), normal(normal) {}

        float& operator[](const int index) {
            return position[index];
        }

        static float distance(KDTreeEntry &a, KDTreeEntry &b) {
            return glm::distance(a.position, b.position);
        }
    };

    using CustomKDTree = KDTree<KDTreeEntry, 3, KDTreeEntry::distance>;

    CustomKDTree* kdTree;
    std::vector<KDTreeEntry> data;
    float alpha;

public:
    ImplicitSurfaceFromPoints(
            std::vector<glm::vec3> &positions,
            std::vector<glm::vec3> &normals,
            float alpha = 0.01f)
    {
        assert(positions.size() == normals.size());
        this->alpha = alpha;

        for (unsigned int i = 0; i < positions.size(); i++) {
            data.push_back(KDTreeEntry(positions[i], glm::normalize(normals[i])));
        }

        kdTree = new CustomKDTree(data, 10);
        kdTree->balance();
    }

    float evaluate(glm::vec3 &position) {
        float sum = 0.0;

        KDTreeEntry _position(position);
        auto entries = kdTree->collectKNearest(_position, 11);
        float maxDistance = getMaxDistance(position, entries);

        for (KDTreeEntry entry : entries) {
            float distance = glm::distance(position, entry.position);
            float weight = maxDistance - distance;
            sum += glm::dot(entry.normal, position - entry.position) / distance;
        }
        return sum;
    }

private:
    float getMaxDistance(glm::vec3 origin, std::vector<KDTreeEntry> points) {
        assert(points.size() > 0);

        float maxDistance = glm::distance(origin, points[0].position);
        for (unsigned int i = 1; i < points.size(); i++) {
            float distance = glm::distance(origin, points[i].position);
            if (distance > maxDistance) maxDistance = distance;
        }

        return maxDistance;
    }
};

bool ImplicitSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    normalShader = new NormalShader();
    shadelessColorShader = new ShadelessColorShader();

    NOffFileData *offData = loadRelNOffResource("horse.off");
    sourcePositions = offData->positions;
    sourceNormals = offData->normals;

    sourcePositionsMesh = new PointCloudMesh<VertexPN>(createVertexPNVector(sourcePositions, sourceNormals));

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
    glEnable(GL_DEPTH_TEST);
    if (displaySourcePoints) drawSourcePoints();
    if (displayGeneratedMesh) drawSurface();
    if (displayVisualizationPoints) drawPointVisualization();
    //drawCurve();
    glDisable(GL_DEPTH_TEST);
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
    shadelessColorShader->bind();
    shadelessColorShader->setViewProj(matViewProj);
}

void ImplicitSurfaceViewer::drawSurface() {
    if (displayAsWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    surface->bindBuffers(normalShader);
    surface->draw();

    if (displayAsWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ImplicitSurfaceViewer::drawPointVisualization() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shadelessColorShader->bind();
    shadelessColorShader->resetModelMatrix();
    implicitSurfacePoints->bindBuffers(shadelessColorShader);
    implicitSurfacePoints->draw();
    glDisable(GL_BLEND);
}

void ImplicitSurfaceViewer::drawSourcePoints() {
    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    sourcePositionsMesh->bindBuffers(normalShader);
    sourcePositionsMesh->draw();
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
    ImGui::Checkbox("Display Visualization Points", &displayVisualizationPoints);
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Generated Mesh", &displayGeneratedMesh);
    if (displayGeneratedMesh) {
        ImGui::Checkbox("Display as Wireframe", &displayAsWireframe);
    }

    recalc |= ImGui::RadioButton("Sphere", (int*)&surfaceSource, SurfaceSource::Sphere); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Genus 2", (int*)&surfaceSource, SurfaceSource::Genus2); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Blobs", (int*)&surfaceSource, SurfaceSource::Blobs); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Points", (int*)&surfaceSource, SurfaceSource::Points);

    if (surfaceSource != SurfaceSource::Points){
        recalc |= ImGui::SliderFloat("Bounding Box Size", &boundingBoxSize, 0.0, 10.0);
    }
    recalc |= ImGui::SliderInt("Resolution", &resolution, 2, 200);


    if (surfaceSource == SurfaceSource::Sphere) {
        recalc |= ImGui::SliderFloat("Radius", &sphereData.radius, 0.0f, 2.0f);
    } else if (surfaceSource == SurfaceSource::Blobs) {
        recalc |= ImGui::SliderFloat3("Blob 1 Position", (float*)&blobData.position1, -2.0f, 2.0f);
        recalc |= ImGui::SliderFloat("Blob 1 Radius", &blobData.radius1, 0.0f, 2.0f);
        recalc |= ImGui::SliderFloat3("Blob 2 Position", (float*)&blobData.position2, -2.0f, 2.0f);
        recalc |= ImGui::SliderFloat("Blob 2 Radius", &blobData.radius2, 0.0f, 2.0f);
    }

    recalc |= ImGui::Checkbox("Flip In- and Outside", &flipInAndOutside);

    if (recalc) {
        updateGeneratedData();
    }

    ImGui::End();
}

void ImplicitSurfaceViewer::updateGeneratedData() {
    TIMEIT("update generated data");

    delete curve;
    curve = nullptr;

    delete surface;
    surface = nullptr;

    delete implicitSurfacePoints;
    implicitSurfacePoints = nullptr;

    BoundingBox<3> box = getBoundingBox();
    ImplicitSurface *implicitSurface = getImplicitSurface();

    std::vector<float> evaluatedValues = evaluateImplicitSurface(
        *implicitSurface, box, flipInAndOutside, resolution, resolution, resolution);

    delete implicitSurface;

    //createImplicitCurve();
    createImplicitSurfaceMesh(evaluatedValues, box);
    createImplicitSurfaceVisualization(evaluatedValues, box);

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

void ImplicitSurfaceViewer::createImplicitSurfaceMesh(std::vector<float> &evaluatedValues, BoundingBox<3> &box) {
    std::vector<glm::vec3> positions = trianglesFromEvaluatedImplicitSurface(
        evaluatedValues, box, resolution, resolution, resolution);

    std::vector<glm::vec3> normals = calculateTriangleVertexNormals(positions);
    surface = new TriangleArrayMesh<VertexPN>(createVertexPNVector(positions, normals));
}

void ImplicitSurfaceViewer::createImplicitSurfaceVisualization(std::vector<float> &evaluatedValues, BoundingBox<3> &box) {
    glm::vec4 innerColor(0.9f, 0.9f, 0.3f, 1.0f);
    glm::vec4 outerColor(0.3f, 0.3f, 0.9f, 1.0f);

    implicitSurfacePoints = coloredPointsFromEvaluatedImplicitSurface(
        evaluatedValues, box, resolution, resolution, resolution,
        innerColor, outerColor);
}

ImplicitSurface *ImplicitSurfaceViewer::getImplicitSurface() {
    if (surfaceSource == SurfaceSource::Sphere) {
        return new ImplicitSphere(sphereData.radius);
    } else if (surfaceSource == SurfaceSource::Genus2) {
        return new ImplicitGenus2Surface();
    } else if (surfaceSource == SurfaceSource::Blobs) {
        ImplicitUnionSurface* unionSurface = new ImplicitUnionSurface();
        unionSurface->addSurface(new ImplicitSphere(blobData.radius1, blobData.position1));
        unionSurface->addSurface(new ImplicitSphere(blobData.radius2, blobData.position2));
        return unionSurface;
    } else if (surfaceSource == SurfaceSource::Points) {
        return new ImplicitSurfaceFromPoints(sourcePositions, sourceNormals);
    }

    assert(false);
    return nullptr;
}

BoundingBox<3> ImplicitSurfaceViewer::getBoundingBox() {
    BoundingBox<3> box;
    if (surfaceSource == SurfaceSource::Points) {
        box = findBoundingBox<glm::vec3, 3>(sourcePositions);
        box.scale(1.1f);
    } else {
        box.min[0] = box.min[1] = box.min[2] = -boundingBoxSize;
        box.max[0] = box.max[1] = box.max[2] =  boundingBoxSize;
    }
    return box;
}