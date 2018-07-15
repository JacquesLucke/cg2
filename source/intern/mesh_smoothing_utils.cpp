#include <iostream>
#include "../mesh_smoothing_utils.hpp"

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