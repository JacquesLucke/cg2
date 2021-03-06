#include <glm/gtx/normal.hpp>
#include "../mesh_utils.hpp"

std::vector<glm::vec3> calculateFaceNormals(
        const std::vector<glm::vec3> &points,
        const std::vector<unsigned int> &indices)
{
    std::vector<glm::vec3> normals;
    for (unsigned int i = 0; i < indices.size(); i += 3) {
        normals.push_back(glm::triangleNormal(
            points[indices[i + 0]],
            points[indices[i + 1]],
            points[indices[i + 2]]));
    }
    return normals;
}

std::vector<glm::vec3> calculateVertexNormals(
        const std::vector<glm::vec3> &points,
        const std::vector<unsigned int> &indices,
        const std::vector<glm::vec3> &faceNormals)
{
    std::vector<glm::vec3> normals;

    for (unsigned int i = 0; i < points.size(); i++) {
        normals.push_back(glm::vec3(0));
    }

    int faceAmount = indices.size() / 3;
    for (int i = 0; i < faceAmount; i++) {
        for (int j = 0; j < 3; j++) {
            normals[indices[3 * i + j]] += faceNormals[i];
        }
    }

    for (unsigned int i = 0; i < points.size(); i++) {
        normals[i] = glm::normalize(normals[i]);
    }

    return normals;
}

std::vector<glm::vec3> calculateVertexNormals(
        const std::vector<glm::vec3> &points,
        const std::vector<unsigned int> &indices)
{
    std::vector<glm::vec3> faceNormals = calculateFaceNormals(points, indices);
    return calculateVertexNormals(points, indices, faceNormals);
}

std::vector<glm::vec3> calculateTriangleVertexNormals(
        const std::vector<glm::vec3> &points)
{
    assert(points.size() % 3 == 0);
    std::vector<glm::vec3> normals;
    for (unsigned int i = 0; i < points.size(); i += 3) {
        glm::vec3 normal = glm::triangleNormal(points[i + 0], points[i + 1], points[i + 2]);
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
    }
    return normals;
}

std::vector<VertexP> createVertexPVector(std::vector<glm::vec3> &positions) {
    std::vector<VertexP> vertices;
    for (unsigned int i = 0; i < positions.size(); i++) {
        vertices.push_back(VertexP(positions[i]));
    }
    return vertices;
}

std::vector<VertexPN> createVertexPNVector(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals) {
    assert(positions.size() == normals.size());

    std::vector<VertexPN> vertices;
    for (unsigned int i = 0; i < positions.size(); i++) {
        vertices.push_back(VertexPN(positions[i], normals[i]));
    }
    return vertices;
}

std::vector<VertexPN> extractPNVerticesFromOffData(OffFileData *offData) {
    std::vector<glm::vec3> faceNormals = calculateFaceNormals(offData->positions, offData->indices);
    std::vector<glm::vec3> vertexNormals = calculateVertexNormals(offData->positions, offData->indices, faceNormals);
    return createVertexPNVector(offData->positions, vertexNormals);
}

std::vector<VertexP> extractPVerticesFromOffData(OffFileData *offData) {
    return createVertexPVector(offData->positions);
}

TriangleGPUMesh<VertexP> *offDataToTriangleMesh_VertexP(OffFileData *offData) {
    std::vector<VertexP> vertices = extractPVerticesFromOffData(offData);
    return new TriangleGPUMesh<VertexP>(vertices, offData->indices);
}

TriangleGPUMesh<VertexPN> *offDataToTriangleMesh_VertexPN(OffFileData *offData) {
    std::vector<VertexPN> vertices = extractPNVerticesFromOffData(offData);
    return new TriangleGPUMesh<VertexPN>(vertices, offData->indices);
}

PointCloudGPUMesh<VertexP> *offDataToPointCloud(OffFileData *offData) {
    std::vector<VertexP> vertices = extractPVerticesFromOffData(offData);
    return new PointCloudGPUMesh<VertexP>(vertices);
}

void appendBoxTriangles(std::vector<VertexP> &vertices, std::vector<unsigned int> &indices, BoundingBox<3> &box) {
    int indexOffset = vertices.size();

    float *limits = (float*)&box;
    for (unsigned int i = 0; i < 8; i++) {
        int xType = (i / 4) % 2;
        int yType = (i / 2) % 2;
        int zType = (i / 1) % 2;
        glm::vec3 point = glm::vec3(
            limits[xType * 3 + 0],
            limits[yType * 3 + 1],
            limits[zType * 3 + 2]);
        vertices.push_back(VertexP(point));
    }

    unsigned int newIndices[6][4] = {
        {0, 1, 2, 3}, {4, 5, 6, 7},
        {0, 1, 4, 5}, {2, 3, 6, 7},
        {0, 2, 4, 6}, {1, 3, 5, 7}
    };

    for (unsigned int i = 0; i < 6; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            indices.push_back(newIndices[i][j] + indexOffset);
        }
        for (unsigned int j = 1; j < 4; j++) {
            indices.push_back(newIndices[i][j] + indexOffset);
        }
    }
}

std::vector<glm::vec3> calcXYGridPoints(int xDiv, int yDiv, BoundingBox<3> box) {
    std::vector<glm::vec3> points;

    for (float x = 0; x < xDiv; x++) {
        for (float y = 0; y < yDiv; y++) {
            points.push_back(glm::vec3(
                box.mapToBox(x / (xDiv - 1.0f), 0),
                box.mapToBox(y / (yDiv - 1.0f), 1),
                0.0f
            ));
        }
    }

    return points;
}

std::vector<EdgeIndices> calcGridEdges(int div1, int div2) {
    std::vector<EdgeIndices> edges;
    for (int i = 0; i < div1; i++) {
        for (int j = 0; j < div2 - 1; j++) {
            edges.push_back(EdgeIndices(
                i * div2 + j + 0,
                i * div2 + j + 1));
        }
    }
    for (int i = 0; i < div2; i++) {
        for (int j = 0; j < div1 - 1; j++) {
            edges.push_back(EdgeIndices(
                (j + 0) * div2 + i,
                (j + 1) * div2 + i));
        }
    }
    return edges;
}

std::vector<unsigned int> calcGridTriangleIndices(int div1, int div2) {
    std::vector<unsigned int> indices;

    for (int i=0; i < div1 - 1; i++) {
        for (int j=0; j < div2 - 1; j++) {
            indices.push_back(i * div2 + j);
            indices.push_back(i * div2 + j + 1);
            indices.push_back(i * div2 + j + 1 + div2);

            indices.push_back(i * div2 + j);
            indices.push_back(i * div2 + j + 1 + div2);
            indices.push_back(i * div2 + j + div2);
        }
    }

    return indices;
}

LinesGPUMesh<VertexP> *generateXYGridLinesMesh(int xDiv, int yDiv, BoundingBox<3> box) {
    std::vector<VertexP> vertices;
    for (float i = 0; i < xDiv; i++) {
        float offset = box.mapToBox(i / (xDiv - 1), 0);
        vertices.push_back(VertexP(glm::vec3(offset, box.min[1], 0)));
        vertices.push_back(VertexP(glm::vec3(offset, box.max[1], 0)));
    }
    for (float i = 0; i < yDiv; i++) {
        float offset = box.mapToBox(i / (yDiv - 1), 1);
        vertices.push_back(VertexP(glm::vec3(box.min[0], offset, 0)));
        vertices.push_back(VertexP(glm::vec3(box.max[0], offset, 0)));
    }
    return new LinesGPUMesh<VertexP>(vertices);
}

LinesGPUMesh<VertexP> *createLineSegmentsMesh(
        std::vector<glm::vec3> starts,
        std::vector<glm::vec3> offsets,
        float scale)
{
    assert(starts.size() == offsets.size());

    std::vector<glm::vec3> linePoints;
    for (unsigned int i = 0; i < starts.size(); i++) {
        linePoints.push_back(starts[i]);
        linePoints.push_back(starts[i] + offsets[i] * scale);
    }
    return new LinesGPUMesh<VertexP>(createVertexPVector(linePoints));
}