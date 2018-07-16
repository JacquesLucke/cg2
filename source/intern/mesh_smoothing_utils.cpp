#include <iostream>
#include "../mesh_smoothing_utils.hpp"
#include "../timer.hpp"

template<typename MeshType>
void smooth_UniformLaplacian(MeshType &mesh, float factor, int steps) {
    TIMEIT("smooth mesh");
    for (int i = 0; i < steps; i++) {
        smooth_UniformLaplacian_OneStep(mesh, factor);
    }
}

template void smooth_UniformLaplacian(Mesh&, float, int);
template void smooth_UniformLaplacian(HalfedgeMesh&, float, int);

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
        averages[i] /= (float)mesh.getVertexDegree(i);
    }

    for (int i = 0; i < vertexAmount; i++) {
        auto position = mesh.getVertexPosition(i);
        mesh.setVertexPosition(i, position + (averages[i] - position) * factor);
    }
}