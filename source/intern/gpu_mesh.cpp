#include <iostream>
#include "../gpu_mesh.hpp"


/* GPUMesh
***********************************************/

template<typename VertexType>
GPUMesh<VertexType>::GPUMesh(const std::vector<VertexType> &vertices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexType), vertices.data(), GL_STATIC_DRAW);
    verticesAmount = vertices.size();
}

template<typename VertexType>
GPUMesh<VertexType>::~GPUMesh() {
    glDeleteBuffers(1, &vbo);
}

template<typename VertexType>
void GPUMesh<VertexType>::bindBuffers(const Shader *shader) {
    bindVertexBuffer(shader);
}

template<>
void GPUMesh<VertexP>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexP), 0);
    glEnableVertexAttribArray(positionAttrLoc);
}

template<>
void GPUMesh<VertexPN>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexPN), 0);
    glEnableVertexAttribArray(positionAttrLoc);

    int normalAttrLoc = shader->getAttributeLocation_Normal();
    glVertexAttribPointer(normalAttrLoc, 3, GL_FLOAT, true, sizeof(VertexPN), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(normalAttrLoc);
}

template<>
void GPUMesh<VertexPC>::bindVertexBuffer(const Shader *shader) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int positionAttrLoc = shader->getAttributeLocation_Position();
    glVertexAttribPointer(positionAttrLoc, 3, GL_FLOAT, false, sizeof(VertexPC), 0);
    glEnableVertexAttribArray(positionAttrLoc);

    int colorAttrLoc = shader->getAttributeLocation_Color();
    glVertexAttribPointer(colorAttrLoc, 4, GL_FLOAT, false, sizeof(VertexPC), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(colorAttrLoc);
}



/* Indexed GPUMesh
****************************************/

template<typename VertexType>
IndexedGPUMesh<VertexType>::IndexedGPUMesh(
        const std::vector<VertexType> &vertices,
        const unsigned int* indices, const int indicesAmount)
            : GPUMesh<VertexType>(vertices)
{
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesAmount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    this->indicesAmount = indicesAmount;
}

template<typename VertexType>
IndexedGPUMesh<VertexType>::~IndexedGPUMesh() {
    glDeleteBuffers(1, &ibo);
}

template<typename VertexType>
void IndexedGPUMesh<VertexType>::bindIndexBuffer() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

template<typename VertexType>
void IndexedGPUMesh<VertexType>::bindBuffers(const Shader *shader) {
    this->bindVertexBuffer(shader);
    this->bindIndexBuffer();
}



/* Triangle GPUMesh
*****************************************/

template<typename VertexType>
void TriangleGPUMesh<VertexType>::draw() {
    glDrawElements(GL_TRIANGLES, this->indicesAmount, GL_UNSIGNED_INT, 0);
}



/*  Wireframe GPUMesh
****************************************/

template<typename VertexType>
void WireframeGPUMesh<VertexType>::draw() {
    glDrawElements(GL_LINES, this->indicesAmount, GL_UNSIGNED_INT, 0);
}



/* Triangle Array GPUMesh
***************************************/

template<typename VertexType>
void TriangleArrayGPUMesh<VertexType>::draw() {
    glDrawArrays(GL_TRIANGLES, 0, this->verticesAmount);
}


/* Point Cloud
***************************************/

template<typename VertexType>
void PointCloudGPUMesh<VertexType>::draw() {
    glDrawArrays(GL_POINTS, 0, this->verticesAmount);
}



/* Lines GPUMesh
****************************************/

template<typename VertexType>
void LinesGPUMesh<VertexType>::draw() {
    glDrawArrays(GL_LINES, 0, this->verticesAmount);
}



/* Explicit Template Instantiation
****************************************/

template class GPUMesh<VertexP>;
template class IndexedGPUMesh<VertexP>;
template class TriangleGPUMesh<VertexP>;
template class PointCloudGPUMesh<VertexP>;
template class WireframeGPUMesh<VertexP>;
template class TriangleArrayGPUMesh<VertexP>;
template class LinesGPUMesh<VertexP>;

template class GPUMesh<VertexPN>;
template class IndexedGPUMesh<VertexPN>;
template class TriangleGPUMesh<VertexPN>;
template class TriangleArrayGPUMesh<VertexPN>;
template class PointCloudGPUMesh<VertexPN>;

template class GPUMesh<VertexPC>;
template class IndexedGPUMesh<VertexPC>;
template class PointCloudGPUMesh<VertexPC>;