#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include <limits>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3 &origin, const glm::vec3 &direction)
        : origin(origin), direction(direction) {}

    float projectPoint(const glm::vec3 &point) const {
        float a = glm::dot(direction, (point - origin));
        float b = glm::dot(direction, direction);
        if (b == 0) return 0;
        return a / b;
    }

    float distanceToParameter(const glm::vec3 &point, float t) {
        return glm::distance(point, origin + t * direction);
    }

    float angleDistanceMetric(const glm::vec3 &point) {
        return glm::dot(direction, glm::normalize(point - origin));
    }

    glm::vec3 getPointWithSmallestAngle(std::vector<glm::vec3> &points) {
        assert(points.size() > 0);

        float minValue = -std::numeric_limits<float>::infinity();
        glm::vec3 closestPoint = points[0];

        for (unsigned int i = 0; i < points.size(); i++) {
            float value = angleDistanceMetric(points[i]);
            if (value > minValue) {
                minValue = value;
                closestPoint = points[i];
            }
        }

        return closestPoint;
    }
};

