#include <unordered_map>
#include <cstdint>
#include <sparsepp/spp.h>
#include "../mesh.hpp"
#include "../utils.hpp"
#include "../timer.hpp"

uint64_t packInts(uint32_t a, uint32_t b) {
    return (uint64_t)a | ((uint64_t)b << 32);
}

HalfEdgeMesh *HalfEdgeMesh::fromTriangles(
        std::vector<glm::vec3> &positions,
        std::vector<unsigned int> &triangleIndices)
{
    TIMEIT("HalfEdgeMesh from triangles");
    assert(triangleIndices.size() % 3 == 0);

    HalfEdgeMesh* mesh = new HalfEdgeMesh();
    int faceAmount = (int)triangleIndices.size() / 3;

    mesh->faces.resize(faceAmount);
    mesh->vertices.resize(positions.size());
    mesh->halfedges.resize(triangleIndices.size());

    for (unsigned int i = 0; i < positions.size(); i++) {
        mesh->vertices[i].position = positions[i];
        mesh->vertices[i].halfedge = -1;
    }


    // utility data structure
    spp::sparse_hash_map<int64_t, int> halfEdgeIndexOf;

    // first pass (also sets up the utility data structure)
    for (int faceIndex = 0; faceIndex < faceAmount; faceIndex++) {
        int offset = faceIndex * 3;
        for (int i = 0; i < 3; i++) {
            int currentHalfEdge = offset + i;
            int nextHalfEdge = offset + (i + 1) % 3;
            unsigned int startVertex = triangleIndices[currentHalfEdge];
            unsigned int endVertex = triangleIndices[nextHalfEdge];

            mesh->halfedges[currentHalfEdge].vertex = startVertex;
            mesh->halfedges[currentHalfEdge].face = faceIndex;
            mesh->halfedges[currentHalfEdge].next = nextHalfEdge;

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