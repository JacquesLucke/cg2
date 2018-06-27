#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "mesh.hpp"
#include "bounding_box.hpp"

class ImplicitSurface {
public:
    virtual ~ImplicitSurface() {}

    virtual float evaluate(glm::vec3 &position) {
        return evaluate(position.x, position.y, position.z);
    }

    virtual float evaluate(float x, float y, float z) {
        auto v = glm::vec3(x, y, z);
        return evaluate(v);
    }
};

std::vector<float> evaluateImplicitSurface(
    ImplicitSurface &surface, BoundingBox<3> box, bool flipInAndOutside,
    int resolutionX, int resolutionY, int resolutionZ);

std::vector<glm::vec3> trianglesFromImplicitSurface(
    ImplicitSurface &surface, BoundingBox<3> box,
    int resolutionX, int resolutionY, int resolutionZ);

std::vector<glm::vec3> trianglesFromEvaluatedImplicitSurface(
    std::vector<float> &evaluatedValues, BoundingBox<3> box,
    int resolutionX, int resolutionY, int resolutionZ);

PointCloudMesh<VertexPC> *coloredPointsFromEvaluatedImplicitSurface(
    std::vector<float> &evaluatedValues, BoundingBox<3> box,
    int resolutionX, int resolutionY, int resolutionZ,
    glm::vec4 innerColor, glm::vec4 outerColor);