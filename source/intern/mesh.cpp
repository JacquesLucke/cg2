#include <iostream>
#include "../mesh.hpp"


/* Mesh
***********************************************/

template<typename VertexType>
Mesh<VertexType>::Mesh(const std::vector<VertexType> &vertices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexType), vertices.data(), GL_STATIC_DRAW);
    verticesAmount = vertices.size();
}

template<typename VertexType>
Mesh<VertexType>::~Mesh() {
    glDeleteBuffers(1, &vbo);
}

template<typename VertexType>
void Mesh<VertexType>::bindBuffers(const Shader *shader) {
    bindVertexBuffer(shader);
}

template<>
void Mesh<VertexP>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexP), 0);
    glEnableVertexAttribArray(positionAttrLoc);
}

template<>
void Mesh<VertexPN>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexPN), 0);
    glEnableVertexAttribArray(positionAttrLoc);

    int normalAttrLoc = shader->getAttributeLocation_Normal();
    glVertexAttribPointer(normalAttrLoc, 3, GL_FLOAT, true, sizeof(VertexPN), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(normalAttrLoc);
}

template<>
void Mesh<VertexPC>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexPC), 0);
    glEnableVertexAttribArray(positionAttrLoc);

    int colorAttrLoc = shader->getAttributeLocation_Color();
    glVertexAttribPointer(colorAttrLoc, 4, GL_FLOAT, false, sizeof(VertexPC), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(colorAttrLoc);
}



/* Indexed Mesh
****************************************/

template<typename VertexType>
IndexedMesh<VertexType>::IndexedMesh(
        const std::vector<VertexType> &vertices,
        const unsigned int* indices, const int indicesAmount)
            : Mesh<VertexType>(vertices)
{
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesAmount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    this->indicesAmount = indicesAmount;
}

template<typename VertexType>
IndexedMesh<VertexType>::~IndexedMesh() {
    glDeleteBuffers(1, &ibo);
}

template<typename VertexType>
void IndexedMesh<VertexType>::bindIndexBuffer() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

template<typename VertexType>
void IndexedMesh<VertexType>::bindBuffers(const Shader *shader) {
    this->bindVertexBuffer(shader);
    this->bindIndexBuffer();
}



/* Triangle Mesh
*****************************************/

template<typename VertexType>
void TriangleMesh<VertexType>::draw() {
    glDrawElements(GL_TRIANGLES, this->indicesAmount, GL_UNSIGNED_INT, 0);
}



/*  Wireframe Mesh
****************************************/

template<typename VertexType>
void WireframeMesh<VertexType>::draw() {
    glDrawElements(GL_LINES, this->indicesAmount, GL_UNSIGNED_INT, 0);
}



/* Triangle Array Mesh
***************************************/

template<typename VertexType>
void TriangleArrayMesh<VertexType>::draw() {
    glDrawArrays(GL_TRIANGLES, 0, this->verticesAmount);
}


/* Point Cloud
***************************************/

template<typename VertexType>
void PointCloudMesh<VertexType>::draw() {
    glDrawArrays(GL_POINTS, 0, this->verticesAmount);
}



/* Lines Mesh
****************************************/

template<typename VertexType>
void LinesMesh<VertexType>::draw() {
    glDrawArrays(GL_LINES, 0, this->verticesAmount);
}



/* Explicit Template Instantiation
****************************************/

template class Mesh<VertexP>;
template class IndexedMesh<VertexP>;
template class TriangleMesh<VertexP>;
template class PointCloudMesh<VertexP>;
template class WireframeMesh<VertexP>;
template class TriangleArrayMesh<VertexP>;
template class LinesMesh<VertexP>;

template class Mesh<VertexPN>;
template class IndexedMesh<VertexPN>;
template class TriangleMesh<VertexPN>;
template class TriangleArrayMesh<VertexPN>;

template class Mesh<VertexPC>;
template class IndexedMesh<VertexPC>;
template class PointCloudMesh<VertexPC>;