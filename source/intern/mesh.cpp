#include <iostream>
#include "../mesh.hpp"


/* Mesh
***********************************************/

template<typename VertexType>
Mesh<VertexType>::Mesh(const std::vector<VertexType> &vertices) {
    this->vertices = vertices;

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexType), vertices.data(), GL_STATIC_DRAW);
}

template<typename VertexType>
Mesh<VertexType>::~Mesh() {
    glDeleteBuffers(1, &vertexBufferID);
}

template<typename VertexType>
void Mesh<VertexType>::bindBuffers(const Shader *shader) {
    bindVertexBuffer(shader);
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



/* Triangle Mesh
*****************************************/

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
void TriangleMesh<VertexType>::bindBuffers(const Shader *shader) {
    bindVertexBuffer(shader);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
}

template<typename VertexType>
void TriangleMesh<VertexType>::draw() {
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}



/* Point Cloud
***************************************/

template<typename VertexType>
void PointCloud<VertexType>::draw() {
    glDrawArrays(GL_POINTS, 0, this->vertices.size());
}



/* Wireframe
****************************************/

template<typename VertexType>
Wireframe<VertexType>::Wireframe(const std::vector<VertexType> &vertices, const std::vector<EdgeIndices> &indices)
    : Mesh<VertexType>(vertices)
{
    this->indices = indices;

    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(EdgeIndices), indices.data(), GL_STATIC_DRAW);
}

template<typename VertexType>
Wireframe<VertexType>::~Wireframe() {
    glDeleteBuffers(1, &indexBufferID);
}

template<typename VertexType>
void Wireframe<VertexType>::bindBuffers(const Shader *shader) {
    bindVertexBuffer(shader);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
}

template<typename VertexType>
void Wireframe<VertexType>::draw() {
    glDrawElements(GL_LINES, indices.size() * 2, GL_UNSIGNED_INT, 0);
}



/* Explicit Template Instantiation
****************************************/

template class Mesh<VertexP>;
template class TriangleMesh<VertexP>;
template class PointCloud<VertexP>;
template class Wireframe<VertexP>;

template class Mesh<VertexPN>;
template class TriangleMesh<VertexPN>;