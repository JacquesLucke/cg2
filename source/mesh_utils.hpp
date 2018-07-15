#pragma once

#include "gpu_mesh.hpp"
#include "resources.hpp"
#include "bounding_box.hpp"

std::vector<glm::vec3> calculateFaceNormals(
    const std::vector<glm::vec3> &points,
    const std::vector<unsigned int> &indices);

std::vector<glm::vec3> calculateVertexNormals(
    const std::vector<glm::vec3> &points,
    const std::vector<unsigned int> &indices,
    const std::vector<glm::vec3> &faceNormals);

std::vector<glm::vec3> calculateVertexNormals(
    const std::vector<glm::vec3> &points,
    const std::vector<unsigned int> &indices);

std::vector<glm::vec3> calculateTriangleVertexNormals(
    const std::vector<glm::vec3> &points);

std::vector<VertexP> createVertexPVector(std::vector<glm::vec3> &positions);
std::vector<VertexPN> createVertexPNVector(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals);

TriangleGPUMesh<VertexP> *offDataToTriangleMesh_VertexP(OffFileData *offData);
TriangleGPUMesh<VertexPN> *offDataToTriangleMesh_VertexPN(OffFileData *offData);
PointCloudGPUMesh<VertexP> *offDataToPointCloud(OffFileData *offData);

void appendBoxTriangles(std::vector<VertexP> &vertices, std::vector<unsigned int> &indices, BoundingBox<3> &box);

std::vector<glm::vec3> calcXYGridPoints(int xDiv, int yDiv, BoundingBox<3> box);
std::vector<EdgeIndices> calcGridEdges(int div1, int div2);
std::vector<unsigned int> calcGridTriangleIndices(int div1, int div2);
LinesGPUMesh<VertexP> *generateXYGridLinesMesh(int xDiv, int yDiv, BoundingBox<3> box);
