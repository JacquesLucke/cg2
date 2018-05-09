#include <iostream>
#include "../mesh.hpp"

template<>
Mesh<VertexP>::Mesh(const std::vector<VertexP> &vertices) {
    this->vertices = vertices;

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexP), vertices.data(), GL_STATIC_DRAW);
}

template<>
void Mesh<VertexP>::bindVertexBuffer(const GLProgram *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

    int positionAttrLoc = shader->getAttributeLocation("position");
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(positionAttrLoc);
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

template<typename VertexType>
void TriangleMesh<VertexType>::draw(const GLProgram *shader) {
    this->bindVertexBuffer(shader);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

template<typename VertexType>
void PointCloud<VertexType>::draw(const GLProgram *shader) {
    this->bindVertexBuffer(shader);
    glDrawArrays(GL_POINTS, 0, this->vertices.size());
}

template class Mesh<VertexP>;
template class TriangleMesh<VertexP>;
template class PointCloud<VertexP>;