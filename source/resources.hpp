#pragma once

#include <string>
#include <vector>

#include "vector.hpp"
#include "shader.hpp"
#include "utils.hpp"

#include "off_files.hpp"

namespace Resources
{
    void init(int argc, char *argv[]);
    std::string getPath(const std::string& resource);
}

std::string loadTextFile(const std::string& path);
OffFileData *loadOffFile(const std::string& path);
GLProgram *loadShader(const std::string& path);

std::string loadRelTextFile(const std::string& path);
OffFileData *loadRelOffFile(const std::string& path);
GLProgram *loadRelShader(const std::string& path);