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

    void bindVertexBuffer(const Shader *shader);
    virtual void bindBuffers(const Shader *shader);
    virtual void draw() = 0;

protected:
    unsigned int vertexBufferID;
    int verticesAmount;
};

template<typename VertexType>
class TriangleMesh : public Mesh<VertexType> {
public:
    TriangleMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices);
    ~TriangleMesh();

    void bindBuffers(const Shader *shader);
    void draw();

protected:
    unsigned int indexBufferID;
    int indicesAmount;
};

template<typename VertexType>
class PointCloudMesh : public Mesh<VertexType> {
public:
    PointCloudMesh(const std::vector<VertexType> &vertices)
        : Mesh<VertexType>(vertices) {}
    ~PointCloudMesh() {}

    void draw();
};

template<typename VertexType>
class WireframeMesh : public Mesh<VertexType> {
public:
    WireframeMesh(const std::vector<VertexType> &vertices, const std::vector<EdgeIndices> &indices);
    ~WireframeMesh();

    void bindBuffers(const Shader *shader);
    void draw();

protected:
    unsigned int indexBufferID;
    int indicesAmount;
};

template<typename VertexType>
class LinesMesh : public Mesh<VertexType> {
public:
    LinesMesh(const std::vector<VertexType> &vertices)
        : Mesh<VertexType>(vertices) {}
    ~LinesMesh() {}

    void draw();
};