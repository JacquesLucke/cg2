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

namespace Resources
{
    void init(int argc, char *argv[]);
    std::string getPath(const std::string &resource);
}

std::string loadTextResource(const std::string &path);
OffFileData *loadOffResource(const std::string &path);
GLProgram *loadShaderResource(const std::string &path);

std::string loadRelTextResource(const std::string &path);
OffFileData *loadRelOffResource(const std::string &path);
GLProgram *loadRelShaderResource(const std::string &path);