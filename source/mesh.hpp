#pragma once

#include <vector>
#include <glm/vec3.hpp>

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
    std::vector<VertexType> vertices;
    unsigned int vertexBufferID;
};

template<typename VertexType>
class TriangleMesh : public Mesh<VertexType> {
public:
    TriangleMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices);
    ~TriangleMesh();

    void bindBuffers(const Shader *shader);
    void draw();

protected:
    std::vector<unsigned int> indices;
    unsigned int indexBufferID;
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
    std::vector<EdgeIndices> indices;
    unsigned int indexBufferID;
};

template<typename VertexType>
class LinesMesh : public Mesh<VertexType> {
public:
    LinesMesh(const std::vector<VertexType> &vertices)
        : Mesh<VertexType>(vertices) {}
    ~LinesMesh() {}

    void draw();
};