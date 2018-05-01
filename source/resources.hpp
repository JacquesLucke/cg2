#pragma once

#include <string>
#include <vector>

#include <glm/vec3.hpp>

#include "shader.hpp"
#include "utils.hpp"
#include "mesh.hpp"

struct OffFileData {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
};

template<typename TVertex>
Mesh<TVertex> *createMesh(const OffFileData *data);

namespace Resources
{
    void init(int argc, char *argv[]);
    std::string getPath(const std::string &resource);
}

std::string loadTextResource(const std::string &path);
OffFileData* loadOffResource(const std::string &path);
GLProgram *loadShaderResource(const std::string &path);

std::string loadRelTextResource(const std::string &path);
OffFileData* loadRelOffResource(const std::string &path);
GLProgram *loadRelShaderResource(const std::string &path);

template<typename TVertex>
Mesh<TVertex> *loadMeshResource(const std::string &path) {
    OffFileData *data = loadOffResource(path);
    if(nullptr == data)
	return nullptr;
    
    Mesh<TVertex> *mesh = createMesh<TVertex>(data);
    delete data;
    return mesh;
}

template<typename TVertex>
Mesh<TVertex> *loadRelMeshResource(const std::string &path) {
    return loadMeshResource<TVertex>(Resources::getPath(path));
}
