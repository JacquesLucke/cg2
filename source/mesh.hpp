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
    virtual int getVertexDegree(int vertexIndex) = 0;
    virtual Mesh *copy() = 0;
    virtual std::vector<unsigned int> getTriangleIndices();
    virtual std::vector<glm::vec3> getVertexPositions();

    virtual std::vector<glm::vec3> neighbours_Vertex_VertexPositions(int vertexIndex);
};

class HalfedgeMesh : public Mesh {
public:
    HalfedgeMesh() {}

    HalfedgeMesh(HalfedgeMesh* source)
        : vertices(source->vertices),
          halfedges(source->halfedges),
          faces(source->faces) {}

    static HalfedgeMesh *fromTriangles(
        std::vector<glm::vec3> &positions,
        std::vector<unsigned int> &triangleIndices);

    std::vector<int> neighbours_Vertex_VertexIndices(int vertexIndex);
    std::vector<int> neighbours_Face_VertexIndices(int faceIndex);
    std::vector<glm::vec3> getVertexPositions();

    HalfedgeMesh *copy() {
        return new HalfedgeMesh(this);
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

    int getVertexDegree(int vertexIndex) {
        return vDegree(vertexIndex);
    }

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
        int degree;
    };

    struct Face {
        int halfedge;
    };

    std::vector<Vertex> vertices;
    std::vector<Halfedge> halfedges;
    std::vector<Face> faces;

    int hNext(int halfedge) { return halfedges[halfedge].next; }
    int hPrev(int halfedge) { return halfedges[halfedge].prev; }
    int hOpposite(int halfedge) { return halfedges[halfedge].opposite; }
    int hEndVertex(int halfedge) { return halfedges[halfedge].endVertex; }

    int vDegree(int vertex) { return vertices[vertex].degree; }
    glm::vec3 vPosition(int vertex) { return vertices[vertex].position; }
    int vOutgoingHalfedge(int vertex) { return vertices[vertex].outgoingHalfedge; }

    int fHalfEdge(int face) { return faces[face].halfedge; }
};