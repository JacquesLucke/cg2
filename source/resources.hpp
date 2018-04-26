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
    std::string loadTextFile(const std::string& path);
    OffFileData *loadOffFile(const std::string& path);
    GLProgram *loadShader(const std::string& path);
}
