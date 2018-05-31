#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <limits>
#include <glm/glm.hpp>

template<int N>
struct BoundingBox {
    std::array<float, N> min;
    std::array<float, N> max;

    float size(int axis) {
        return max[axis] - min[axis];
    }

    float maxsize() {
        float max = 0;
        for (int i = 0; i < N; i++) {
            if (size(i) > max) max = size(i);
        }
        return max;
    }

    float mapBetween0And1(float value, int axis) {
        return (value - min[axis]) / size(axis);
    }

    float mapToBox(float value, int axis) {
        return value * size(axis) + min[axis];
    }

    friend std::ostream& operator<<(std::ostream& stream, const BoundingBox& box) {
        stream << "[";
        for (size_t i = 0; i < N; i++) {
            stream << "(" << box.min[i] << ", " << box.max[i] << ")";
            if (i < N - 1) stream << ", ";
        }
        stream << "]";
        return stream;
    }
};

template<typename T, int ndim>
BoundingBox<ndim> findBoundingBox(T* points, unsigned int length) {
    BoundingBox<ndim> box;

    for (int i = 0; i < ndim; i++) {
        box.min[i] = +std::numeric_limits<float>::infinity();
        box.max[i] = -std::numeric_limits<float>::infinity();
    }

    for (unsigned int i = 0; i < length; i++) {
        for (int j = 0; j < ndim; j++) {
            float value = points[i][j];
            if (value < box.min[j]) box.min[j] = value;
            if (value > box.max[j]) box.max[j] = value;
        }
    }

    return box;
}

template<typename T, int ndim>
BoundingBox<ndim> findBoundingBox(std::vector<T> points) {
    return findBoundingBox<T, ndim>(points.data(), points.size());
}