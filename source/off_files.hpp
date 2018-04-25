#pragma once

#include <vector>
#include <string>
#include "vector.hpp"

struct OffFileData {
    std::vector<Vector<3>> positions;
};

OffFileData *readOffFile(std::string path);