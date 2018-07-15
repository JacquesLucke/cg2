#include <vector>
#include <glm/glm.hpp>

class Mesh {
public:
    virtual ~Mesh() {}
};

class HalfEdgeMesh : public Mesh {
public:
    static HalfEdgeMesh *fromTriangles(
        std::vector<glm::vec3> &positions,
        std::vector<unsigned int> &triangleIndices);

private:
    struct Halfedge {
        int vertex;
        int face;
        int next;
        int opposite;
    };

    struct Vertex {
        glm::vec3 position;
        int halfedge;
    };

    struct Face {
        int halfedge;
    };

    std::vector<Vertex> vertices;
    std::vector<Halfedge> halfedges;
    std::vector<Face> faces;
};