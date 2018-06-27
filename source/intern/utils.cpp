#include <thread>
#include <fstream>
#include <sstream>
#include "../utils.hpp"

inline int getMedianIndex(int left, int right) {
    return (left + right) / 2;
}

int getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}

float getMaxDistance(glm::vec3 origin, std::vector<glm::vec3> points) {
    assert(points.size() > 0);

    float maxDistance = glm::distance(origin, points[0]);
    for (unsigned int i = 1; i < points.size(); i++) {
        float distance = glm::distance(origin, points[i]);
        if (distance > maxDistance) maxDistance = distance;
    }

    return maxDistance;
}