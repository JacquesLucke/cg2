#include "../implicit_curve.hpp"
#include "../mesh_utils.hpp"

inline bool haveSameSign(float a, float b) {
    return a*b >= 0.0f;
}

float edgeVertexPosition(float pos1, float pos2, float value1, float value2) {
    if (haveSameSign(value1, value2)) {
        return (pos1 + pos2) / 2;
    }

    value1 = std::abs(value1);
    value2 = std::abs(value2);
    return (pos1 * value2 + pos2 * value1) / (value1 + value2);
}

char lineTable[16][2] = {
    {-1, -1},
    { 3, -1},
    { 0, -1},
    { 5, -1},
    { 1, -1},
    { 1,  3},
    { 4, -1},
    { 2, -1},

    { 2, -1},
    { 4, -1},
    { 0,  2},
    { 1, -1},
    { 5, -1},
    { 0, -1},
    { 3, -1},
    {-1, -1}
};

static char edgeTable[6][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    {0, 2},
    {1, 3}
};

LinesMesh<VertexP> *linesFromImplicitCurve(ImplicitCurve &curve, BoundingBox<2> box, int resolution) {
    std::vector<glm::vec3> positions;
    float fResolution = (float)resolution;

    for (int x = 0; x < resolution; x++) {
        float x0 = box.mapToBox((x + 0) / fResolution, 0);
        float x1 = box.mapToBox((x + 1) / fResolution, 0);

        for (int y = 0; y < resolution; y++) {
            float y0 = box.mapToBox((y + 0) / fResolution, 1);
            float y1 = box.mapToBox((y + 1) / fResolution, 1);

            float gridValues[4] = {
                curve.evaluate(x0, y0),
                curve.evaluate(x1, y0),
                curve.evaluate(x1, y1),
                curve.evaluate(x0, y1)
            };

            glm::vec2 edgePoints[4] = {
                glm::vec2(edgeVertexPosition(x0, x1, gridValues[0], gridValues[1]), y0),
                glm::vec2(x1, edgeVertexPosition(y0, y1, gridValues[1], gridValues[2])),
                glm::vec2(edgeVertexPosition(x0, x1, gridValues[3], gridValues[2]), y1),
                glm::vec2(x0, edgeVertexPosition(y0, y1, gridValues[0], gridValues[3]))
            };

            char lookupIndex = 0;
            for (int i = 0; i < 4; i++) {
                lookupIndex |= (gridValues[i] < 0) << i;
            }

            char *lines = lineTable[(unsigned int)lookupIndex];

            for (int i = 0; i < 2; i++) {
                char line = lines[i];
                if (line == -1) break;

                char *edgeIndices = edgeTable[(unsigned int)line];
                for (int j = 0; j < 2; j++){
                    positions.push_back(
                    	glm::vec3(edgePoints[(unsigned int)edgeIndices[j]], 0));
                }
            }
        }
    }

    return new LinesMesh<VertexP>(createVertexPVector(positions));
}
