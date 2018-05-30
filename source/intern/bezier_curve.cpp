#include "../bezier_curve.hpp"

glm::vec3 mixVectors(glm::vec3 a, glm::vec3 b, float t) {
    return a * (1 - t) + b * t;
}

void evaluateDeCasteljau(std::vector<glm::vec3> controlPoints, float t,
        glm::vec3* outPosition, glm::vec3* outTangent)
{
    // controlPoints vector is reused
    for (unsigned int pass = 0; pass < controlPoints.size() - 2; pass++) {
        for (unsigned int j = 0; j < controlPoints.size() - pass - 1; j++) {
            controlPoints[j] = mixVectors(controlPoints[j], controlPoints[j+1], t);
        }
    }

    if (outPosition != nullptr){
        *outPosition = mixVectors(controlPoints[0], controlPoints[1], t);
    }
    if (outTangent != nullptr) {
        *outTangent = controlPoints[1] - controlPoints[0];
    }
}

void BezierCurve::evaluate(float t, glm::vec3* outPosition, glm::vec3* outTangent) {
    evaluateDeCasteljau(controls, t, outPosition, outTangent);
}

std::vector<glm::vec3> BezierCurve::getPositionSamples(int amount) {
    std::vector<glm::vec3> positions;
    for (int i = 0; i < amount; i++) {
        float t = i / (amount - 1.0f);
        glm::vec3 position;
        evaluate(t, &position);
        positions.push_back(position);
    }
    return positions;
}

std::vector<glm::vec3> evaluateMultipleBezierCurves(std::vector<BezierCurve> curves, float t) {
    std::vector<glm::vec3> positions;
    for (unsigned int i = 0; i < curves.size(); i++) {
        glm::vec3 position;
        curves[i].evaluate(t, &position);
        positions.push_back(position);
    }
    return positions;
}