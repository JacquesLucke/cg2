#include "../random.hpp"

std::vector<glm::vec3> generateRandomPoints(unsigned int amount, int seed) {
    std::vector<glm::vec3> points;
    for (unsigned int i = 0; i < amount; i++) {
        int s = seed + i * 3;
        points.push_back(glm::vec3(
            randomFloat_Range(s + 0, 1),
            randomFloat_Range(s + 1, 1),
            randomFloat_Range(s + 2, 1)));
    }
    return points;
}