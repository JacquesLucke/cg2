#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

int getCpuCoreCount();
std::string readFile(std::string path);
float getMaxDistance(glm::vec3 origin, std::vector<glm::vec3> points);