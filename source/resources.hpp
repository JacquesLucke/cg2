#pragma once

#include <string>
#include <vector>

#include <glm/vec3.hpp>

#include "shader.hpp"
#include "utils.hpp"
#include "gpu_mesh.hpp"

struct OffFileData {
    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;
};

struct NOffFileData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
};

namespace Resources
{
    void init(int argc, char *argv[]);
    std::string getPath(const std::string &resource);
}

std::string loadTextResource(const std::string &path);
OffFileData *loadOffResource(const std::string &path);
NOffFileData *loadNOffResource(const std::string &path);
GLProgram *loadShaderResource(const std::string &path);

std::string loadRelTextResource(const std::string &path);
OffFileData *loadRelOffResource(const std::string &path);
NOffFileData *loadRelNOffResource(const std::string &path);
GLProgram *loadRelShaderResource(const std::string &path);