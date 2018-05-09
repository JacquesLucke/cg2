#include "../mesh.hpp"

template<>
Mesh<VertexP>::Mesh(const std::vector<VertexP> &vertices) {
    this->vertices = vertices;

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexP), vertices.data(), GL_STATIC_DRAW);
}

template<typename VertexType>
Mesh<VertexType>::~Mesh() {
    glDeleteBuffers(1, &vertexBufferID);
}

template<typename VertexType>
TriangleMesh<VertexType>::TriangleMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices)
    : Mesh<VertexType>(vertices)
{
    this->indices = indices;

    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

template<>
TriangleMesh<VertexP>::~TriangleMesh() {
    glDeleteBuffers(1, &indexBufferID);
}

template<>
void TriangleMesh<VertexP>::draw(const GLProgram *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

template class Mesh<VertexP>;
template class TriangleMesh<VertexP>;