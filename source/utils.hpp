#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <Eigen/Dense>

int getCpuCoreCount();
std::string readFile(std::string path);
float getMaxDistance(glm::vec3 origin, std::vector<glm::vec3> points);

typedef float (*WeightFunction)(float distance);
typedef Eigen::VectorXf (*LeastSquaresSolverFunction)(Eigen::MatrixXf &A, Eigen::VectorXf &b);

float wendland(float d);

enum LeastSquaresSolver {
    SVD, QR, Normal
};

Eigen::VectorXf solveLeastSquares_SVD(Eigen::MatrixXf &A, Eigen::VectorXf &b);
Eigen::VectorXf solveLeastSquares_QR(Eigen::MatrixXf &A, Eigen::VectorXf &b);
Eigen::VectorXf solveLeastSquares_Normal(Eigen::MatrixXf &A, Eigen::VectorXf &b);
LeastSquaresSolverFunction getLeastSquaresSolver(LeastSquaresSolver solverType);