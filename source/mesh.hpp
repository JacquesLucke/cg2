#pragma once

#include <vector>
#include <glm/vec3.hpp>

#include "ogl.hpp"
#include "shader.hpp"

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

    virtual void draw(const GLProgram *shader) = 0;

protected:
    std::vector<VertexType> vertices;
    unsigned int vertexBufferID;
};

template<typename VertexType>
class TriangleMesh : public Mesh<VertexType> {
public:
    TriangleMesh(const std::vector<VertexType> &vertices, const std::vector<unsigned int> &indices);
    ~TriangleMesh();

    void draw(const GLProgram *shader);

protected:
    std::vector<unsigned int> indices;
    unsigned int indexBufferID;
};