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
class PointCloud : public Mesh<VertexType> {
public:
    PointCloud(const std::vector<VertexType> &vertices)
        : Mesh<VertexType>(vertices) {}
    ~PointCloud() {}

    void draw();
};
