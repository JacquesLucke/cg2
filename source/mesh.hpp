#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "ogl.hpp"
#include "shaders.hpp"

struct VertexP {
    VertexP(glm::vec3 position)
	    : position(position) {}

    glm::vec3 position;
};

struct VertexPN {
    VertexPN(glm::vec3 position, glm::vec3 normal)
        : position(position), normal(normal) {}

    glm::vec3 position;
    glm::vec3 normal;
};

struct VertexPC {
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
class Mesh {
public:
    Mesh(const std::vector<VertexType> &vertices);
    ~Mesh();

    virtual void bindBuffers(const Shader *shader);
    void bindVertexBuffer(const Shader *shader);
    virtual void draw() = 0;

protected:
    unsigned int vbo;
    int verticesAmount;
};

template<typename VertexType>
class IndexedMesh : public Mesh<VertexType> {
public:
    IndexedMesh(const std::vector<VertexType> &vertices, const unsigned int* indices, const int indicesAmount);
    IndexedMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices)
        : IndexedMesh(vertices, indices.data(), (int)indices.size()) {}
    ~IndexedMesh();

    void bindIndexBuffer();
    void bindBuffers(const Shader *shader);

protected:
    unsigned int ibo;
    int indicesAmount;
};

template<typename VertexType>
class TriangleMesh : public IndexedMesh<VertexType> {
public:
    TriangleMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices)
        : IndexedMesh<VertexType>(vertices, indices) {}

    void draw();
};

template<typename VertexType>
class WireframeMesh : public IndexedMesh<VertexType> {
public:
    WireframeMesh(const std::vector<VertexType> &vertices, const std::vector<EdgeIndices> &indices)
        : IndexedMesh<VertexType>(vertices, (unsigned int*)indices.data(), (int)indices.size() * 2) {}

    void draw();
};

template<typename VertexType>
class PointCloudMesh : public Mesh<VertexType> {
public:
    PointCloudMesh(const std::vector<VertexType> &vertices)
        : Mesh<VertexType>(vertices) {}

    void draw();
};

template<typename VertexType>
class LinesMesh : public Mesh<VertexType> {
public:
    LinesMesh(const std::vector<VertexType> &vertices)
        : Mesh<VertexType>(vertices) {}
    void draw();
};