#include <thread>
#include <fstream>
#include <sstream>
#include "../utils.hpp"

inline int getMedianIndex(int left, int right) {
    return (left + right) / 2;
}

int getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}

float getMaxDistance(glm::vec3 origin, std::vector<glm::vec3> points) {
    assert(points.size() > 0);

    float maxDistance = glm::distance(origin, points[0]);
    for (unsigned int i = 1; i < points.size(); i++) {
        float distance = glm::distance(origin, points[i]);
        if (distance > maxDistance) maxDistance = distance;
    }

    return maxDistance;
}

float wendland(float d) {
    if (d > 1) return 0.0f;
    return pow(1 - d, 4) * (4 * d + 1);
}

Eigen::VectorXf solveLeastSquares_SVD(Eigen::MatrixXf &A, Eigen::VectorXf &b) {
    return A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
}

Eigen::VectorXf solveLeastSquares_QR(Eigen::MatrixXf &A, Eigen::VectorXf &b) {
    return A.colPivHouseholderQr().solve(b);
}

Eigen::VectorXf solveLeastSquares_Normal(Eigen::MatrixXf &A, Eigen::VectorXf &b) {
    return (A.transpose() * A).ldlt().solve(A.transpose() * b);
}

LeastSquaresSolverFunction getLeastSquaresSolver(LeastSquaresSolver solverType) {
    switch (solverType) {
        case LeastSquaresSolver::SVD: return solveLeastSquares_SVD;
        case LeastSquaresSolver::QR: return solveLeastSquares_QR;
        case LeastSquaresSolver::Normal: return solveLeastSquares_Normal;
        default: assert(false); return NULL;
    }
}