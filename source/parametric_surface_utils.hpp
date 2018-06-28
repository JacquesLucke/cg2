#include <vector>
#include <glm/glm.hpp>
#include "kdtree.hpp"
#include "utils.hpp"

inline float distanceSquaredFunction(glm::vec3 &a, glm::vec3 &b) {
    float xDiff = a.x - b.x;
    float yDiff = a.y - b.y;
    return xDiff * xDiff + yDiff * yDiff;
}

inline float distanceFunction(glm::vec3 &a, glm::vec3 &b) {
    return sqrt(distanceSquaredFunction(a, b));
}

using KDTreeVec3_2D = KDTree<glm::vec3, 2, distanceFunction>;

enum RadiusSelectionMode {
    Radius,
    KNearest
};

struct RadiusSelectionInfo {
    static RadiusSelectionInfo useRadius(float radius) {
        RadiusSelectionInfo info;
        info.mode = RadiusSelectionMode::Radius;
        info.radius = radius;
        return info;
    }

    static RadiusSelectionInfo useKNearest(int k) {
        RadiusSelectionInfo info;
        info.mode = RadiusSelectionMode::KNearest;
        info.k = k;
        return info;
    }

    RadiusSelectionMode mode = RadiusSelectionMode::Radius;
    float radius = 0.1f;
    int k = 20;
};

void setDataWithMovingLeastSquares(
    std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals,
    KDTreeVec3_2D *kdTree, RadiusSelectionInfo radiusSelectionInfo,
    LeastSquaresSolver solverType, bool parallel);

void evaluateDeCasteljau(std::vector<glm::vec3> controlPoints, float t,
    glm::vec3* outPosition, glm::vec3* outTangent);