#include <iostream>
#include "../mesh.hpp"

template<typename VertexType>
Mesh<VertexType>::Mesh(const std::vector<VertexType> &vertices) {
    this->vertices = vertices;

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexType), vertices.data(), GL_STATIC_DRAW);
}

template<>
void Mesh<VertexP>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexP), 0);
    glEnableVertexAttribArray(positionAttrLoc);
}

template<>
void Mesh<VertexPN>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexPN), 0);
    glEnableVertexAttribArray(positionAttrLoc);

    int normalAttrLoc = shader->getAttributeLocation_Normal();
    glVertexAttribPointer(normalAttrLoc, 3, GL_FLOAT, true, sizeof(VertexPN), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(normalAttrLoc);
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

template<typename VertexType>
TriangleMesh<VertexType>::~TriangleMesh() {
    glDeleteBuffers(1, &indexBufferID);
}

template<typename VertexType>
void TriangleMesh<VertexType>::draw(const Shader *shader) {
    shader->bind();
    this->bindVertexBuffer(shader);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

template<typename VertexType>
void PointCloud<VertexType>::draw(const Shader *shader) {
    shader->bind();
    this->bindVertexBuffer(shader);
    glDrawArrays(GL_POINTS, 0, this->vertices.size());
}

template class Mesh<VertexP>;
template class TriangleMesh<VertexP>;
template class PointCloud<VertexP>;

template class Mesh<VertexPN>;
template class TriangleMesh<VertexPN>;