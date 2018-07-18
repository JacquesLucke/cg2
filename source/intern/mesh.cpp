#include <unordered_map>
#include <cstdint>
#include <sparsepp/spp.h>
#include "../mesh.hpp"
#include "../utils.hpp"
#include "../timer.hpp"

std::vector<glm::vec3> Mesh::neighbours_Vertex_VertexPositions(int vertexIndex) {
    std::vector<glm::vec3> positions;
    for (auto index : neighbours_Vertex_VertexIndices(vertexIndex)) {
        positions.push_back(getVertexPosition(index));
    }
    return positions;
}

std::vector<glm::vec3> Mesh::getVertexPositions() {
    std::vector<glm::vec3> positions;
    for (int i = 0; i < getVertexAmount(); i++) {
        positions.push_back(getVertexPosition(i));
    }
    return positions;
}

std::vector<unsigned int> Mesh::getTriangleIndices() {
    std::vector<unsigned int> indices;
    for (int i = 0; i < getFaceAmount(); i++) {
        auto faceIndices = neighbours_Face_VertexIndices(i);
        assert(faceIndices.size() == 3);
        for (int j = 0; j < 3; j++) {
            indices.push_back(faceIndices[j]);
        }
    }
    return indices;
}

uint64_t packInts(uint32_t a, uint32_t b) {
    return (uint64_t)a | ((uint64_t)b << 32);
}

HalfedgeMesh *HalfedgeMesh::fromTriangles(
        std::vector<glm::vec3> &positions,
        std::vector<unsigned int> &triangleIndices)
{
    TIMEIT("HalfedgeMesh from triangles");
    assert(triangleIndices.size() % 3 == 0);

    HalfedgeMesh* mesh = new HalfedgeMesh();
    int faceAmount = (int)triangleIndices.size() / 3;

    mesh->faces.resize(faceAmount);
    mesh->vertices.resize(positions.size());
    mesh->halfedges.resize(triangleIndices.size());

    for (unsigned int i = 0; i < positions.size(); i++) {
        mesh->vertices[i].position = positions[i];
        mesh->vertices[i].outgoingHalfedge = -1;
    }


    // utility data structure
    spp::sparse_hash_map<int64_t, int> halfEdgeIndexOf;

    // first pass (also sets up the utility data structure)
    for (int faceIndex = 0; faceIndex < faceAmount; faceIndex++) {
        int offset = faceIndex * 3;
        for (int i = 0; i < 3; i++) {
            int currentHalfEdge = offset + i;
            int nextHalfEdge = offset + (i + 1) % 3;
            int prevHalfEdge = offset + (i + 2) % 3;

            unsigned int startVertex = triangleIndices[currentHalfEdge];
            unsigned int endVertex = triangleIndices[nextHalfEdge];

            mesh->halfedges[currentHalfEdge].endVertex = endVertex;
            mesh->halfedges[currentHalfEdge].face = faceIndex;
            mesh->halfedges[currentHalfEdge].next = nextHalfEdge;
            mesh->halfedges[currentHalfEdge].prev = prevHalfEdge;

            mesh->vertices[startVertex].outgoingHalfedge = currentHalfEdge;
            halfEdgeIndexOf[packInts(startVertex, endVertex)] = currentHalfEdge;
        }
        mesh->faces[faceIndex].halfedge = offset;
    }

    // second pass, fill in opposites using separate data structure
    for (int faceIndex = 0; faceIndex < faceAmount; faceIndex++) {
        int offset = faceIndex * 3;
        for (int i = 0; i < 3; i++) {
            int currentHalfEdge = offset + i;
            int nextHalfEdge = offset + (i + 1) % 3;
            unsigned int startVertex = triangleIndices[currentHalfEdge];
            unsigned int endVertex = triangleIndices[nextHalfEdge];

            mesh->halfedges[currentHalfEdge].opposite = halfEdgeIndexOf[packInts(endVertex, startVertex)];
        }
    }

    return mesh;
}

std::vector<glm::vec3> HalfedgeMesh::getVertexPositions() {
    std::vector<glm::vec3> positions;
    for (auto vertex : vertices) {
        positions.push_back(vertex.position);
    }
    return positions;
}

std::vector<int> HalfedgeMesh::neighbours_Vertex_VertexIndices(int vertexIndex) {
    std::vector<int> indices;
    int halfedge = vOutgoingHalfedge(vertexIndex);
    int stop = halfedge;
    do {
        indices.push_back(hEndVertex(halfedge));
        halfedge = hNext(hOpposite(halfedge));
    } while (halfedge != stop);
    return indices;
}

std::vector<int> HalfedgeMesh::neighbours_Face_VertexIndices(int faceIndex) {
    std::vector<int> indices;
    int halfedge = fHalfEdge(faceIndex);
    int stop = halfedge;
    do {
        indices.push_back(hEndVertex(halfedge));
        halfedge = hNext(halfedge);
    } while (halfedge != stop);
    return indices;
}

int HalfedgeMesh::getVertexDegree(int vertexIndex) {
    int degree = 0;
    int halfedge = vOutgoingHalfedge(vertexIndex);
    int stop = halfedge;
    do {
        degree++;
        halfedge = hNext(hOpposite(halfedge));
    } while (halfedge != stop);
    return degree;
}

int HalfedgeMesh::hNext(int halfedge) {
    return halfedges[halfedge].next;
}

int HalfedgeMesh::hPrev(int halfedge) {
    return halfedges[halfedge].prev;
}

int HalfedgeMesh::hOpposite(int halfedge) {
    return halfedges[halfedge].opposite;
}

int HalfedgeMesh::hEndVertex(int halfedge) {
    return halfedges[halfedge].endVertex;
}

int HalfedgeMesh::vOutgoingHalfedge(int vertex) {
    return vertices[vertex].outgoingHalfedge;
}

glm::vec3 HalfedgeMesh::vPosition(int vertex) {
    return vertices[vertex].position;
}

int HalfedgeMesh::fHalfEdge(int face) {
    return faces[face].halfedge;
}
