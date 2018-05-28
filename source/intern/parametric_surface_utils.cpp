#include <thread>
#include <algorithm>
#include <Eigen/Dense>
#include "../parametric_surface_utils.hpp"
#include "../timer.hpp"
#include "../utils.hpp"

#define COEFF_AMOUNT 6

struct Coefficients {
    float c_1;
    float c_x, c_y;
    float c_xx, c_xy, c_yy;

    float evaluate(float x, float y) {
        return c_1 + c_x*x + c_y*y + c_xx*x*x + c_xy*x*y + c_yy*y*y;
    }

    glm::vec3 derivative(float x, float y) {
        glm::vec3 tangentX(1, 0, c_x + 2*c_xx*x + c_xy*y);
        glm::vec3 tangentY(0, 1, c_y + 2*c_yy*y + c_xy*x);
        glm::vec3 normal = glm::cross(tangentX, tangentY);
        return glm::normalize(normal);
    }

    friend std::ostream& operator<<(std::ostream &os, const Coefficients &c);
};

std::ostream& operator<<(std::ostream &os, const Coefficients &c) {
    os << c.c_1 << " + " << c.c_x << "x + " << c.c_y << "y + " << c.c_xx << "x^2 + " << c.c_xy << "xy + " << c.c_yy << "y^2";
    return os;
}

typedef float (*WeightFunction)(float distance);
typedef Eigen::VectorXf (*LeastSquaresSolverFunction)(Eigen::MatrixXf &A, Eigen::VectorXf &b);

Coefficients weightedLeastSquares(std::vector<glm::vec3> &points, std::vector<float> &weights, LeastSquaresSolverFunction solver) {
    assert(points.size() == weights.size());

    Eigen::MatrixXf A(points.size(), COEFF_AMOUNT);
    Eigen::VectorXf b = Eigen::VectorXf(points.size());

    for (unsigned int i = 0; i < points.size(); i++) {
        float x = points[i].x,  y = points[i].y, z = points[i].z;
        float w = weights[i];

        A(i, 0) = 1 * w;
        A(i, 1) = x * w;
        A(i, 2) = y * w;
        A(i, 3) = x * x * w;
        A(i, 4) = x * y * w;
        A(i, 5) = y * y * w;
        b[i] = z * w;
    }

    Eigen::VectorXf result = solver(A, b);

    Coefficients coeffs;
    memcpy(&coeffs, &result[0], sizeof(Coefficients));
    return coeffs;
}



std::vector<float> calcWeights(std::vector<glm::vec3> &points, glm::vec3 position, WeightFunction f, float radius) {
    std::vector<float> weights;
    for (unsigned int i = 0; i < points.size(); i++) {
        weights.push_back(f(glm::distance(glm::vec2(points[i]), glm::vec2(position)) / radius));
    }
    return weights;
}

float weightFunction_Wendland(float d) {
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

void setDataWithMovingLeastSquares_Part(
        std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals,
        int start, int end, KDTreeVec3_2D *kdTree, float radius,
        LeastSquaresSolver solverType)
{
    for (int i = start; i < end; i++) {
        std::vector<glm::vec3> pointsToConsider = kdTree->collectInRadius(points[i], radius);
        if (pointsToConsider.size() > 0) {
            auto weights = calcWeights(pointsToConsider, points[i], weightFunction_Wendland, radius);
            Coefficients coeffs = weightedLeastSquares(pointsToConsider, weights, getLeastSquaresSolver(solverType));
            points[i].z = coeffs.evaluate(points[i].x, points[i].y);
            normals[i] = coeffs.derivative(points[i].x, points[i].y);
        } else {
            normals[i] = glm::vec3(0, 0, 1);
        }
    }
}

void setDataWithMovingLeastSquares(
        std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals,
        KDTreeVec3_2D *kdTree, float radius, LeastSquaresSolver solverType, bool parallel)
{
    TIMEIT("moving least squares")
    assert(points.size() == normals.size());

    if (parallel) {
        int threadCount = getCpuCoreCount() + 1;
        int chunkSize = (int)ceil((float)points.size() / (float)threadCount);
        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount; i++) {
            int start = i * chunkSize;
            int end = std::min(start + chunkSize, (int)points.size());
            threads.push_back(
                std::thread(setDataWithMovingLeastSquares_Part,
                    std::ref(points), std::ref(normals),
                    start, end, kdTree, radius, solverType)
            );
        }
        while (!threads.empty()) {
            threads.back().join();
            threads.pop_back();
        }
    } else {
        setDataWithMovingLeastSquares_Part(points, normals, 0, points.size(), kdTree, radius, solverType);
    }
}