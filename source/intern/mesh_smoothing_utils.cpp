#include <iostream>
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <glm/gtx/vector_angle.hpp>
#include "../mesh_smoothing_utils.hpp"
#include "../timer.hpp"
#include "../utils.hpp"

template<typename MeshType>
void smooth_UniformLaplacian(MeshType &mesh, float factor, int steps) {
    TIMEIT("smooth mesh - uniform");
    for (int i = 0; i < steps; i++) {
        smooth_UniformLaplacian_OneStep(mesh, factor);
    }
}

template<typename MeshType>
void smooth_CotanLaplacian(MeshType &mesh, float factor, int steps, bool doImplicit) {
    TIMEIT("smooth mesh - explicit cotan");
    for (int i = 0; i < steps; i++) {
        smooth_CotanLaplacian_OneStep(mesh, factor, doImplicit);
    }
}

template void smooth_UniformLaplacian(Mesh&, float, int);
template void smooth_UniformLaplacian(HalfedgeMesh&, float, int);
template void smooth_CotanLaplacian(Mesh&, float, int, bool);
template void smooth_CotanLaplacian(HalfedgeMesh&, float, int, bool);

template<>
void smooth_UniformLaplacian_OneStep(Mesh &mesh, float factor) {
    std::vector<glm::vec3> averages;
    int vertexAmount = mesh.getVertexAmount();
    averages.resize(vertexAmount);

    for (int i = 0; i < vertexAmount; i++) {
        auto neighbours = mesh.neighbours_Vertex_VertexPositions(i);
        for (auto position : neighbours) {
            averages[i] += position;
        }
        averages[i] /= (float)neighbours.size();
    }

    for (int i = 0; i < vertexAmount; i++) {
        auto position = mesh.getVertexPosition(i);
        mesh.setVertexPosition(i, position + (averages[i] - position) * factor);
    }
}

template<>
void smooth_UniformLaplacian_OneStep(HalfedgeMesh &mesh, float factor) {
    std::vector<glm::vec3> averages;
    int vertexAmount = mesh.getVertexAmount();
    averages.resize(vertexAmount);

    for (auto halfedge : mesh.halfedges) {
        averages[halfedge.endVertex] += mesh.vPosition(mesh.hEndVertex(halfedge.prev));
    }

    for (int i = 0; i < vertexAmount; i++) {
        averages[i] /= (float)mesh.vDegree(i);
    }

    for (int i = 0; i < vertexAmount; i++) {
        auto position = mesh.vPosition(i);
        mesh.setVertexPosition(i, position + (averages[i] - position) * factor);
    }
}

typedef Eigen::Triplet<float> Triplet;
typedef Eigen::SparseMatrix<float> SparseMatrix;

SparseMatrix generateCotanLaplaceMatrix(Mesh &mesh, bool useArea = true) {
    int vertexAmount = mesh.getVertexAmount();
    std::vector<Triplet> tripletsM;
    std::vector<Triplet> tripletsD;

    for (int i = 0; i < vertexAmount; i++) {
        auto vertex = mesh.getVertexPosition(i);
        auto Ni = mesh.neighbours_Vertex_VertexIndices(i);
        int degree = Ni.size();

        float weightSum = 0.0f;
        float areaSum = 0.0f;

        for (int j = 0; j < degree; j++) {
            auto vLeft = mesh.getVertexPosition(Ni[mod(j + 1, degree)]);
            auto vRight = mesh.getVertexPosition(Ni[mod(j - 1, degree)]);
            auto iNeighbour = Ni[j];
            auto vActive = mesh.getVertexPosition(iNeighbour);

            auto leftDir1 = vActive - vLeft;
            auto leftDir2 = vertex - vLeft;
            auto rightDir1 = vActive - vRight;
            auto rightDir2 = vertex - vRight;

            float angleLeft = glm::angle(leftDir1, leftDir2);
            float angleRight = glm::angle(rightDir1, rightDir2);
            float weight = cotan(angleLeft) + cotan(angleRight);
            weightSum += weight;

            float leftArea = calcTriangleArea(leftDir1, leftDir2) / 3.0f;
            areaSum += leftArea;

            tripletsM.push_back(Triplet(i, iNeighbour, weight));
        }

        tripletsM.push_back(Triplet(i, i, -weightSum));
        tripletsD.push_back(Triplet(i, i, 1 / (2 * areaSum)));
    }

    SparseMatrix M(vertexAmount, vertexAmount);
    SparseMatrix D(vertexAmount, vertexAmount);
    M.setFromTriplets(tripletsM.begin(), tripletsM.end());
    D.setFromTriplets(tripletsD.begin(), tripletsD.end());

    if (useArea) return D * M;
    return M;
}

template<typename MeshType>
void smooth_CotanLaplacian_OneStep(MeshType &mesh, float factor, bool doImplicit) {
    SparseMatrix L = generateCotanLaplaceMatrix(mesh);

    int vertexAmount = mesh.getVertexAmount();
    Eigen::VectorXf coordsX(vertexAmount);
    Eigen::VectorXf coordsY(vertexAmount);
    Eigen::VectorXf coordsZ(vertexAmount);

    for (int i = 0; i < vertexAmount; i++) {
        glm::vec3 pos = mesh.getVertexPosition(i);
        coordsX[i] = pos[0];
        coordsY[i] = pos[1];
        coordsZ[i] = pos[2];
    }

    Eigen::VectorXf newCoordsX;
    Eigen::VectorXf newCoordsY;
    Eigen::VectorXf newCoordsZ;

    if (doImplicit) {
        std::vector<Triplet> tripletsId;
        for (int i = 0; i < vertexAmount; i++) {
            tripletsId.push_back(Triplet(i, i, 1));
        }
        SparseMatrix Id(vertexAmount, vertexAmount);
        Id.setFromTriplets(tripletsId.begin(), tripletsId.end());

        SparseMatrix A = Id - factor * L;
        Eigen::SparseLU<SparseMatrix> solver;
        solver.compute(A);
        newCoordsX = solver.solve(coordsX);
        newCoordsY = solver.solve(coordsY);
        newCoordsZ = solver.solve(coordsZ);
    } else {
        newCoordsX = coordsX + factor * L * coordsX;
        newCoordsZ = coordsZ + factor * L * coordsZ;
        newCoordsY = coordsY + factor * L * coordsY;
    }

    for (int i = 0; i < vertexAmount; i++) {
        mesh.setVertexPosition(i, glm::vec3(
            newCoordsX[i], newCoordsY[i], newCoordsZ[i]
        ));
    }
}

template<typename MeshType>
std::vector<Eigen::VectorXf> calcCotanLaplacianEigenVectors(MeshType &mesh) {
    TIMEIT("calc eigenvectors");
    SparseMatrix sparseL = generateCotanLaplaceMatrix(mesh, false);
    Eigen::MatrixXf L(sparseL);

    Eigen::EigenSolver<Eigen::MatrixXf> solver(L);

    std::vector<Eigen::VectorXf> eigenvectors;
    std::vector<float> eigenvalues;

    for (int i = 0; i < mesh.getVertexAmount(); i++) {
        Eigen::VectorXcf eigenvector(solver.eigenvectors().col(i));
        Eigen::VectorXf ei = eigenvector.real();
        eigenvectors.push_back(ei);
        eigenvalues.push_back(solver.eigenvalues()[i].real());
    }

    std::vector<int> idx;
    for (unsigned int i = 0; i < eigenvalues.size(); i++) idx.push_back(i);

    std::sort(idx.begin(), idx.end(),
        [&eigenvalues](int i1, int i2) {
            return eigenvalues[i2] < eigenvalues[i1];
        });

    std::vector<Eigen::VectorXf> sortedEigenvectors;
    for (int i : idx) {
        sortedEigenvectors.push_back(eigenvectors[i]);
    }

    return sortedEigenvectors;
}

template<typename MeshType>
void smooth_Spectral(MeshType &mesh, std::vector<Eigen::VectorXf> &eigenvectors, int k) {
    TIMEIT("smooth spectral");

    int vertexAmount = mesh.getVertexAmount();
    Eigen::VectorXf coordsX(vertexAmount);
    Eigen::VectorXf coordsY(vertexAmount);
    Eigen::VectorXf coordsZ(vertexAmount);

    for (int i = 0; i < vertexAmount; i++) {
        glm::vec3 pos = mesh.getVertexPosition(i);
        coordsX[i] = pos[0];
        coordsY[i] = pos[1];
        coordsZ[i] = pos[2];
    }

    Eigen::VectorXf newCoordsX(vertexAmount);
    Eigen::VectorXf newCoordsY(vertexAmount);
    Eigen::VectorXf newCoordsZ(vertexAmount);
    newCoordsX.fill(0);
    newCoordsY.fill(0);
    newCoordsZ.fill(0);

    for (int i = 0; i < k; i++) {
        auto ei = eigenvectors[i];
        newCoordsX += (coordsX.transpose() * ei) * ei;
        newCoordsY += (coordsY.transpose() * ei) * ei;
        newCoordsZ += (coordsZ.transpose() * ei) * ei;
    }

    for (int i = 0; i < vertexAmount; i++) {
        mesh.setVertexPosition(i, glm::vec3(
            newCoordsX[i], newCoordsY[i], newCoordsZ[i]
        ));
    }
}

template std::vector<Eigen::VectorXf> calcCotanLaplacianEigenVectors(Mesh&);
template std::vector<Eigen::VectorXf> calcCotanLaplacianEigenVectors(HalfedgeMesh&);
template void smooth_Spectral(Mesh&, std::vector<Eigen::VectorXf>&, int);
template void smooth_Spectral(HalfedgeMesh&, std::vector<Eigen::VectorXf>&, int);