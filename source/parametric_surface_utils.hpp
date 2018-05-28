#include <vector>
#include <glm/glm.hpp>
#include "kdtree.hpp"

inline float distanceSquaredFunction(glm::vec3 &a, glm::vec3 &b) {
    float xDiff = a.x - b.x;
    float yDiff = a.y - b.y;
    return xDiff * xDiff + yDiff * yDiff;
}

inline float distanceFunction(glm::vec3 &a, glm::vec3 &b) {
    return sqrt(distanceSquaredFunction(a, b));
}

using KDTreeVec3_2D = KDTree<glm::vec3, 2, distanceFunction>;

enum LeastSquaresSolver {
    SVD, QR, Normal
};

void setDataWithMovingLeastSquares(
    std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals,
    KDTreeVec3_2D *kdTree, float radius, LeastSquaresSolver solverType, bool parallel);

void evaluateDeCasteljau(std::vector<glm::vec3> controlPoints, float t,
    glm::vec3* outPosition, glm::vec3* outTangent);