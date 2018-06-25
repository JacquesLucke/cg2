#pragma once

#include <glm/glm.hpp>

#include "mesh.hpp"
#include "bounding_box.hpp"

class ImplicitSurface {
public:
    virtual ~ImplicitSurface() {}

    virtual float evaluate(glm::vec3 &position) {
        return evaluate(position.x, position.y, position.z);
    }

    virtual float evaluate(float x, float y, float z) {
        return evaluate(glm::vec3(x, y, z));
    }
};

TriangleMesh<VertexP> *trianglesFromImplicitSurface(
    ImplicitSurface &surface, BoundingBox<3> box,
    int resolutionX, int resolutionY, int resolutionZ);