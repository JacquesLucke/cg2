#pragma once
#include <vector>
#include <glm/glm.hpp>

class BezierCurve {
public:
    BezierCurve(std::vector<glm::vec3> controls)
        : controls(controls) {}

    void evaluate(float t, glm::vec3* outPosition, glm::vec3* outTangent = nullptr);
    std::vector<glm::vec3> getPositionSamples(int amount);

private:
    std::vector<glm::vec3> controls;
};

std::vector<glm::vec3> evaluateMultipleBezierCurves(std::vector<BezierCurve> &curves, float t);
std::vector<glm::vec3> gridFromBezierCurves(std::vector<BezierCurve> &curves, int uDiv, int vDiv);