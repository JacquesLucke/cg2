#include "../mesh.hpp"

Mesh<VertexP>::Mesh(const std::vector<VertexP> &vertices, const std::vector<unsigned int> &indices)
    : Mesh_B(vertices.size(), indices.size()), vertexBuffer(0), indexBuffer(0)
{
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexP), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

Mesh<VertexP>::~Mesh()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
}

void Mesh<VertexP>::bindBuffers()
{
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
}

void Mesh<VertexP>::unbindBuffers()
{
    glDisableVertexAttribArray(0); 
}
