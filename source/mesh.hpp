#pragma once

#include <vector>
#include <glm/glm.hpp>

class Mesh {
public:
    virtual ~Mesh() {}

    virtual std::vector<int> neighbours_Vertex_VertexIndices(int vertexIndex) = 0;
    virtual std::vector<int> neighbours_Face_VertexIndices(int faceIndex) = 0;
    virtual glm::vec3 getVertexPosition(int vertexIndex) = 0;
    virtual void setVertexPosition(int vertexIndex, glm::vec3 position) = 0;
    virtual int getVertexAmount() = 0;
    virtual int getFaceAmount() = 0;
    virtual Mesh *copy() = 0;

    std::vector<glm::vec3> neighbours_Vertex_VertexPositions(int vertexIndex);
};

class HalfEdgeMesh : public Mesh {
public:
    HalfEdgeMesh() {}

    HalfEdgeMesh(HalfEdgeMesh* source)
        : vertices(source->vertices),
          halfedges(source->halfedges),
          faces(source->faces) {}

    static HalfEdgeMesh *fromTriangles(
        std::vector<glm::vec3> &positions,
        std::vector<unsigned int> &triangleIndices);

    std::vector<int> neighbours_Vertex_VertexIndices(int vertexIndex);
    std::vector<int> neighbours_Face_VertexIndices(int faceIndex);

    HalfEdgeMesh *copy() {
        return new HalfEdgeMesh(this);
    }

    glm::vec3 getVertexPosition(int vertexIndex) {
        return vertices[vertexIndex].position;
    }

    void setVertexPosition(int vertexIndex, glm::vec3 position) {
        vertices[vertexIndex].position = position;
    }

    int getVertexAmount() {
        return (int)vertices.size();
    }

    int getFaceAmount() {
        return (int)faces.size();
    }

private:
    struct Halfedge {
        int endVertex;
        int face;
        int next;
        int prev;
        int opposite;
    };

    struct Vertex {
        glm::vec3 position;
        int outgoingHalfedge;
    };

    struct Face {
        int halfedge;
    };

    std::vector<Vertex> vertices;
    std::vector<Halfedge> halfedges;
    std::vector<Face> faces;

    int hNext(int halfedge);
    int hPrev(int halfedge);
    int hOpposite(int halfedge);
    int hEndVertex(int halfedge);
    int vOutgoingHalfedge(int vertex);
    int fHalfEdge(int face);
};