#pragma once

#include <glm/glm.hpp>

#include "gpu_mesh.hpp"
#include "bounding_box.hpp"

class ImplicitCurve {
public:
    virtual ~ImplicitCurve() {}

    virtual float evaluate(glm::vec2 &position) {
        return evaluate(position.x, position.y);
    }

    virtual float evaluate(float x, float y) {
        auto v = glm::vec2(x, y);
        return evaluate(v);
    }
};

LinesGPUMesh<VertexP> *linesFromImplicitCurve(
    ImplicitCurve &curve, BoundingBox<2> box, int resolution);
