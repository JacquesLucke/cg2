#include "../ogl.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Eigen/Dense>

#include "../implicit_surface_viewer.hpp"
#include "../mesh_utils.hpp"
#include "../timer.hpp"
#include "../utils.hpp"


/* Specific Implicit Curves
*********************************************/

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


/* Specific Implicit Surfaces
*********************************************/

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


/* Implicit Surfaces based on Points and Normals
*****************************************************/

float wendland(float d) {
    if (d > 1) return 0.0f;
    return pow(1 - d, 4) * (4 * d + 1);
}

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
    int minPointAmount;
    float minRadius;

public:
    ImplicitSurfaceFromPoints(
            std::vector<glm::vec3> &positions,
            std::vector<glm::vec3> &normals,
            int minPointAmount = 10, float minRadius = 0.0f)
            : minPointAmount(minPointAmount), minRadius(minRadius)
    {
        assert(positions.size() == normals.size());

        for (unsigned int i = 0; i < positions.size(); i++) {
            data.push_back(KDTreeEntry(positions[i], glm::normalize(normals[i])));
        }

        kdTree = new CustomKDTree(data, 10);
        kdTree->balance();
    }

    ~ImplicitSurfaceFromPoints() {
        delete kdTree;
    }

    float evaluate(glm::vec3 &position) {
        KDTreeEntry _position(position);
        auto entries = kdTree->collectInRadius(_position, minRadius);

        if ((int)entries.size() < minPointAmount) {
            entries = kdTree->collectKNearest(_position, minPointAmount);
        }

        float maxDistance = getMaxDistance(position, entries);

        float sum = 0.0;
        float weightSum = 0.0f;

        for (KDTreeEntry entry : entries) {
            glm::vec3 difference = position - entry.position;
            float distance = glm::length(difference);
            float weight = wendland(distance / maxDistance);
            if (distance > 0.000001) {
                sum += weight * glm::dot(entry.normal, difference / distance);
            }
            weightSum += weight;
        }
        if (weightSum > 0) {
            return sum / weightSum;
        } else {
            return 0.0;
        }
    }

private:
    float getMaxDistance(glm::vec3 origin, std::vector<KDTreeEntry> &points) {
        assert(points.size() > 0);

        float maxDistance = glm::distance(origin, points[0].position);
        for (unsigned int i = 1; i < points.size(); i++) {
            float distance = glm::distance(origin, points[i].position);
            if (distance > maxDistance) maxDistance = distance;
        }

        return maxDistance;
    }
};

class ImplicitSurfaceFromPoints2 : public ImplicitSurface {
    struct KDTreeEntry {
        glm::vec3 position;
        float value;

        KDTreeEntry(glm::vec3 position = glm::vec3(0), float value = 0)
            : position(position), value(value) {}

        float& operator[](const int index) {
            return position[index];
        }

        static float distance(KDTreeEntry &a, KDTreeEntry &b) {
            return glm::distance(a.position, b.position);
        }
    };

    using CustomKDTree = KDTree<KDTreeEntry, 3, KDTreeEntry::distance>;

    CustomKDTree* epsilonKDTree;
    KDTreeVec3* zeroKDTree;
    std::vector<KDTreeEntry> data;
    std::vector<glm::vec3> zeroPositions;
    float radius;

public:
    ImplicitSurfaceFromPoints2(
            std::vector<glm::vec3> &positions,
            std::vector<glm::vec3> &normals,
            float radius)
            : radius(radius), zeroPositions(positions)
    {
        assert(positions.size() == normals.size());

        zeroKDTree = new KDTreeVec3(zeroPositions, 10);
        zeroKDTree->balance();

        for (unsigned int i = 0; i < positions.size(); i++) {
            glm::vec3 position = positions[i];
            glm::vec3 offset = glm::normalize(normals[i]);

            float epsilon1 = findEpsilon(position, offset, 0.01f);
            data.push_back(KDTreeEntry(position + epsilon1 * offset, epsilon1));

            float epsilon2 = findEpsilon(position, offset, -0.01f);
            data.push_back(KDTreeEntry(position + epsilon2 * offset, epsilon2));
        }

        epsilonKDTree = new CustomKDTree(data, 10);
        epsilonKDTree->balance();
    }

    ~ImplicitSurfaceFromPoints2() {
        delete epsilonKDTree;
        delete zeroKDTree;
    }

    float findEpsilon(glm::vec3 &position, glm::vec3 &offset, float startEpsilon) {
        float epsilon = startEpsilon;
        while (zeroKDTree->getClosestPoint(position + offset * epsilon) != position) {
            epsilon /= 2.0f;
        }
        return epsilon;
    }

    float evaluate(glm::vec3 &position) {
        KDTreeEntry _position(position);
        auto entries = epsilonKDTree->collectInRadius(_position, radius);

        if (entries.size() < 4) {
            return epsilonKDTree->getClosestPoint(_position).value;
        }

        Eigen::MatrixXf A(entries.size(), 10);
        Eigen::VectorXf b = Eigen::VectorXf(entries.size());

        for (unsigned int i = 0; i < entries.size(); i++) {
            float x = entries[i].position.x;
            float y = entries[i].position.y;
            float z = entries[i].position.z;
            float value = entries[i].value;
            float weight = wendland(1 - glm::distance(position, entries[i].position) / radius);

            A(i, 0) = 1 * weight;
            A(i, 1) = x * weight;
            A(i, 2) = y * weight;
            A(i, 3) = z * weight;
            A(i, 4) = x * y * weight;
            A(i, 5) = x * z * weight;
            A(i, 6) = y * z * weight;
            A(i, 7) = x * x * weight;
            A(i, 8) = y * y * weight;
            A(i, 9) = z * z * weight;

            b[i] = value * weight;
        }

        Eigen::VectorXf coeffs = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);

        float _x = position.x;
        float _y = position.y;
        float _z = position.z;

        return (coeffs[0] +
                coeffs[1] * _x      + coeffs[2] * _y      + coeffs[3] * _z,
                coeffs[4] * _x * _y + coeffs[5] * _x * _z + coeffs[6] * _y * _z,
                coeffs[7] * _x * _x + coeffs[8] * _y * _y + coeffs[9] * _z * _z);
    }
};


/* Start of actual Window Controller
********************************************/

bool ImplicitSurfaceViewer::onSetup() {
    flatShader = new FlatShader();
    normalShader = new NormalShader();
    shadelessColorShader = new ShadelessColorShader();
    phongShader = new BlinnPhongShader();

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
    phongShader->bind();
    phongShader->setViewProj(matViewProj);
}

void ImplicitSurfaceViewer::drawSurface() {
    if (displayAsWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    surface->bindBuffers(normalShader);
    surface->draw();

    // phongShader->bind();
    // phongShader->resetModelMatrix();
    // phongShader->setAmbientColor(0.1f, 0.1f, 0.1f);
    // phongShader->setDiffuseColor(0.9f, 0.7f, 0.3f);
    // phongShader->setSpecularColor(1, 1, 1);
    // phongShader->setMagnitude(4);
    // phongShader->setLightPosition(lightPosition);
    // phongShader->setCameraPosition(camera->camera->eye);

    surface->bindBuffers(phongShader);
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
    glPointSize(5);
    normalShader->bind();
    normalShader->resetModelMatrix();
    normalShader->setBrightness(1);
    sourcePositionsMesh->bindBuffers(normalShader);
    sourcePositionsMesh->draw();
    glPointSize(1);
}

void ImplicitSurfaceViewer::onRenderUI() {
    ImGui::Begin("CG2");

    bool recalc = false;
    ImGui::Checkbox("Display Source Points", &displaySourcePoints);
    ImGui::Checkbox("Display Visualization Points", &displayVisualizationPoints);
    ImGui::Checkbox("Display Generated Mesh", &displayGeneratedMesh);
    if (displayGeneratedMesh) {
        ImGui::Checkbox("Display as Wireframe", &displayAsWireframe);
    }

    recalc |= ImGui::RadioButton("Sphere", (int*)&surfaceSource, SurfaceSource::Sphere); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Genus 2", (int*)&surfaceSource, SurfaceSource::Genus2); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Blobs", (int*)&surfaceSource, SurfaceSource::Blobs); ImGui::SameLine();
    recalc |= ImGui::RadioButton("Points", (int*)&surfaceSource, SurfaceSource::Points);

    recalc |= ImGui::SliderInt("Resolution", &resolution, 2, 200);

    if (surfaceSource != SurfaceSource::Points){
        recalc |= ImGui::SliderFloat("Bounding Box Size", &boundingBoxSize, 0.0, 10.0);
    }

    if (surfaceSource == SurfaceSource::Sphere) {
        recalc |= ImGui::SliderFloat("Radius", &sphereData.radius, 0.0f, 2.0f);
    } else if (surfaceSource == SurfaceSource::Blobs) {
        recalc |= ImGui::SliderFloat3("Blob 1 Position", (float*)&blobData.position1, -2.0f, 2.0f);
        recalc |= ImGui::SliderFloat("Blob 1 Radius", &blobData.radius1, 0.0f, 2.0f);
        recalc |= ImGui::SliderFloat3("Blob 2 Position", (float*)&blobData.position2, -2.0f, 2.0f);
        recalc |= ImGui::SliderFloat("Blob 2 Radius", &blobData.radius2, 0.0f, 2.0f);
    } else if (surfaceSource == SurfaceSource::Points) {
        recalc |= ImGui::SliderInt("Min Point Amount", &pointsData.minPointAmount, 1, 50);
        recalc |= ImGui::SliderFloat("Min Relative Radius", &pointsData.minRelativeRadius, 0.0f, 0.3f);
        recalc |= ImGui::SliderFloat("Bounding Box Factor", &pointsData.boundingBoxFactor, 1.0f, 2.0f);
    }

    recalc |= ImGui::Checkbox("Flip In- and Outside", &flipInAndOutside);

    // ImGui::Separator();
    // if (ImGui::SliderFloat3("Relative Light Position", (float*)&relativeLightPosition, -2, 3)) {
    //     updateLightPosition();
    // }

    if (recalc) {
        updateGeneratedData();
    }

    ImGui::End();
}

void ImplicitSurfaceViewer::updateGeneratedData() {
    TIMEIT("update generated data");

    delete surface;
    surface = nullptr;

    delete implicitSurfacePoints;
    implicitSurfacePoints = nullptr;

    boundingBox = getBoundingBox();
    ImplicitSurface *implicitSurface = getImplicitSurface();

    std::vector<float> evaluatedValues = evaluateImplicitSurface(
        *implicitSurface, boundingBox, flipInAndOutside, resolution, resolution, resolution);

    delete implicitSurface;

    createImplicitSurfaceMesh(evaluatedValues);
    createImplicitSurfaceVisualization(evaluatedValues);
}

void ImplicitSurfaceViewer::createImplicitSurfaceMesh(std::vector<float> &evaluatedValues) {
    std::vector<glm::vec3> positions = trianglesFromEvaluatedImplicitSurface(
        evaluatedValues, boundingBox, resolution, resolution, resolution);

    std::vector<glm::vec3> normals = calculateTriangleVertexNormals(positions);
    surface = new TriangleArrayMesh<VertexPN>(createVertexPNVector(positions, normals));
}

void ImplicitSurfaceViewer::createImplicitSurfaceVisualization(std::vector<float> &evaluatedValues) {
    glm::vec4 innerColor(0.9f, 0.9f, 0.3f, 1.0f);
    glm::vec4 outerColor(0.3f, 0.3f, 0.9f, 1.0f);

    implicitSurfacePoints = coloredPointsFromEvaluatedImplicitSurface(
        evaluatedValues, boundingBox, resolution, resolution, resolution,
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
        return new ImplicitSurfaceFromPoints(
            sourcePositions, sourceNormals,
            pointsData.minPointAmount, pointsData.minRelativeRadius * boundingBox.size(0));
    }

    assert(false);
    return nullptr;
}

BoundingBox<3> ImplicitSurfaceViewer::getBoundingBox() {
    BoundingBox<3> box;
    if (surfaceSource == SurfaceSource::Points) {
        box = findBoundingBox<glm::vec3, 3>(sourcePositions);
        box.scale(pointsData.boundingBoxFactor);
    } else {
        box.min[0] = box.min[1] = box.min[2] = -boundingBoxSize;
        box.max[0] = box.max[1] = box.max[2] =  boundingBoxSize;
    }
    return box;
}

void ImplicitSurfaceViewer::updateLightPosition() {
    lightPosition = glm::vec3(
        boundingBox.mapToBox(relativeLightPosition.x, 0),
        boundingBox.mapToBox(relativeLightPosition.y, 1),
        boundingBox.mapToBox(relativeLightPosition.z, 2)
    );
}