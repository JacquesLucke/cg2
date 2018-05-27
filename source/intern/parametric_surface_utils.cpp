#include <Eigen/Dense>
#include "../parametric_surface_utils.hpp"
#include "../timer.hpp"

#define COEFF_AMOUNT 6

struct Coefficients {
    float c_1;
    float c_x, c_y;
    float c_xx, c_xy, c_yy;

    float evaluate(float x, float y) {
        return c_1 + c_x*x + c_y*y + c_xx*x*x + c_xy*x*y + c_yy*y*y;
    }

    friend std::ostream& operator<<(std::ostream &os, const Coefficients &c);
};

std::ostream& operator<<(std::ostream &os, const Coefficients &c) {
    os << c.c_1 << " + " << c.c_x << "x + " << c.c_y << "y + " << c.c_xx << "x^2 + " << c.c_xy << "xy + " << c.c_yy << "y^2";
    return os;
}

Coefficients weightedLeastSquares(std::vector<glm::vec3> &points, std::vector<float> &weights) {
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

    Eigen::VectorXf result = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);

    Coefficients coeffs;
    memcpy(&coeffs, &result[0], sizeof(Coefficients));
    return coeffs;
}

typedef float (*WeightFunction)(float distance);

std::vector<float> calcWeights(std::vector<glm::vec3> &points, glm::vec3 position, WeightFunction f, float radius) {
    std::vector<float> weights;
    for (unsigned int i = 0; i < points.size(); i++) {
        weights.push_back(f(glm::distance(glm::vec2(points[i]), glm::vec2(position))));
    }
    return weights;
}

float weightFunction_Wendland(float d) {
    if (d > 1) return 0.0f;
    return pow(1 - d, 4) * (4 * d + 1);
}

float getZBasedOnMovingLeastSquares(glm::vec3 position, std::vector<glm::vec3> &points, float radius) {
    auto weights = calcWeights(points, position, weightFunction_Wendland, radius);
    Coefficients coeffs = weightedLeastSquares(points, weights);
    return coeffs.evaluate(position.x, position.y);
}

void setZValuesWithMovingLeastSquares(std::vector<glm::vec3> &points, KDTreeVec3_2D *kdTree, float radius) {
    TIMEIT("moving least squares")
    for (unsigned int i = 0; i < points.size(); i++) {
        std::vector<glm::vec3> pointsToConsider = kdTree->collectInRadius(points[i], radius);
        if (pointsToConsider.size() > 0) {
            points[i].z = getZBasedOnMovingLeastSquares(points[i], pointsToConsider, radius);
        }
    }
}