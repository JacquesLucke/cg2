#pragma once

#include <vector>
#include <glm/vec3.hpp>

#include "ogl.hpp"

struct VertexP
{
    VertexP(const glm::vec3 pos)
	: position(pos)
    {}
    
    glm::vec3 position;
};

template<typename TVertex>
class Mesh_B
{
public:
    Mesh_B(unsigned int vertexCount, unsigned int indexCount)
	: vertexCount(vertexCount), indexCount(indexCount)
    {}
    
    Mesh_B(const Mesh_B&) = delete;

    unsigned int getVertexCount() const;
    unsigned int getIndexCount() const;

private:
    unsigned int vertexCount;
    unsigned int indexCount;
};


template<typename TVertex>
unsigned int Mesh_B<TVertex>::getVertexCount() const { return vertexCount; }

template<typename TVertex>
unsigned int Mesh_B<TVertex>::getIndexCount() const { return indexCount; }

template<typename TVertex>
class Mesh : public Mesh_B<TVertex>
{
public:
    Mesh(const std::vector<TVertex> &vertices, const std::vector<unsigned int> &indices);
    ~Mesh();

    void bindBuffers();
    void unbindBuffers();
};

template<>
class Mesh<VertexP> : public Mesh_B<VertexP>
{
public: 
    Mesh(const std::vector<VertexP> &vertices, const std::vector<unsigned int> &indices);
    ~Mesh();

    void bindBuffers();
    void unbindBuffers();
private:
    GLuint vertexBuffer;
    GLuint indexBuffer;
};
