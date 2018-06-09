#pragma once

#include <glm/glm.hpp>

#include "mesh.hpp"
#include "bounding_box.hpp"

class ImplicitCurve {
public:
    virtual float evaluate(glm::vec2 &position) {
        return evaluate(position.x, position.y);
    }

    virtual float evaluate(float x, float y) {
        return evaluate(glm::vec2(x, y));
    }
};

LinesMesh<VertexP> *linesFromImplicitCurve(ImplicitCurve &curve, BoundingBox<2> box, int resolution);