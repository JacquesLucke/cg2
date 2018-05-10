#pragma once

#include <vector>
#include <glm/vec3.hpp>

#include "ogl.hpp"
#include "shaders.hpp"

struct VertexP {
    VertexP(const glm::vec3 pos)
	: position(pos)
    {}

    glm::vec3 position;
};

template<typename VertexType>
class Mesh {
public:
    Mesh(const std::vector<VertexType> &vertices);
    ~Mesh();

    void bindVertexBuffer(const Shader *shader);
    virtual void draw(const Shader *shader) = 0;

protected:
    std::vector<VertexType> vertices;
    unsigned int vertexBufferID;
};

template<typename VertexType>
class TriangleMesh : public Mesh<VertexType> {
public:
    TriangleMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices);
    ~TriangleMesh();

    void draw(const Shader *shader);

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

    void draw(const Shader *shader);
};
