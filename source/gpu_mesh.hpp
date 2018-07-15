#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "ogl.hpp"
#include "shaders.hpp"

struct VertexP {
    VertexP() {}
    VertexP(glm::vec3 position)
	    : position(position) {}

    glm::vec3 position;
};

struct VertexPN {
    VertexPN() {}
    VertexPN(glm::vec3 position, glm::vec3 normal)
        : position(position), normal(normal) {}

    glm::vec3 position;
    glm::vec3 normal;
};

struct VertexPC {
    VertexPC() {}
    VertexPC(glm::vec3 position, glm::vec4 color)
        : position(position), color(color) {}

    glm::vec3 position;
    glm::vec4 color;
};

struct EdgeIndices {
    EdgeIndices(unsigned int v1, unsigned int v2)
        : v1(v1), v2(v2) {}

    unsigned int v1, v2;
};

template<typename VertexType>
class GPUMesh {
public:
    GPUMesh(const std::vector<VertexType> &vertices);
    virtual ~GPUMesh();

    virtual void bindBuffers(const Shader *shader);
    void bindVertexBuffer(const Shader *shader);
    virtual void draw() = 0;

protected:
    unsigned int vbo;
    int verticesAmount;
};

template<typename VertexType>
class IndexedGPUMesh : public GPUMesh<VertexType> {
public:
    IndexedGPUMesh(const std::vector<VertexType> &vertices, const unsigned int* indices, const int indicesAmount);
    IndexedGPUMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices)
        : IndexedGPUMesh(vertices, indices.data(), (int)indices.size()) {}
    ~IndexedGPUMesh();

    void bindIndexBuffer();
    void bindBuffers(const Shader *shader);

protected:
    unsigned int ibo = 0;
    int indicesAmount = 0;
};

template<typename VertexType>
class TriangleGPUMesh : public IndexedGPUMesh<VertexType> {
public:
    TriangleGPUMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices)
        : IndexedGPUMesh<VertexType>(vertices, indices) {}

    void draw();
};

template<typename VertexType>
class WireframeGPUMesh : public IndexedGPUMesh<VertexType> {
public:
    WireframeGPUMesh(const std::vector<VertexType> &vertices, const std::vector<EdgeIndices> &indices)
        : IndexedGPUMesh<VertexType>(vertices, (unsigned int*)indices.data(), (int)indices.size() * 2) {}

    void draw();
};

template<typename VertexType>
class TriangleArrayGPUMesh : public GPUMesh<VertexType> {
public:
    TriangleArrayGPUMesh(const std::vector<VertexType> &vertices)
        : GPUMesh<VertexType>(vertices) {}

    void draw();
};

template<typename VertexType>
class PointCloudGPUMesh : public GPUMesh<VertexType> {
public:
    PointCloudGPUMesh(const std::vector<VertexType> &vertices)
        : GPUMesh<VertexType>(vertices) {}

    void draw();
};

template<typename VertexType>
class LinesGPUMesh : public GPUMesh<VertexType> {
public:
    LinesGPUMesh(const std::vector<VertexType> &vertices)
        : GPUMesh<VertexType>(vertices) {}
    void draw();
};
