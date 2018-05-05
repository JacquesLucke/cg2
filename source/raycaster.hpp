#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    float minDistance;
    float maxDistance;

    Ray(const glm::vec3 &origin, const glm::vec3 &direction, float minDistance, float maxDistance);
    Ray(const glm::vec3 &origin, const glm::vec3 &direction);
};

std::vector<glm::vec3> matchPoints(const Ray &ray, const std::vector<glm::vec3> &points, float radius);
