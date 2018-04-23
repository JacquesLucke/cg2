#pragma once

#include "vector.h"
#include <array>

template<int N>
struct BoundingBox {
    std::array<float, N> min;
    std::array<float, N> max;

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

template<int N>
BoundingBox<N> findBoundingBox(Vector<N> *points, int length) {
    TIMEIT("Bounding Boxes")
    BoundingBox<N> box;
    for (int i = 0; i < N; i++) {
        box.min[i] = +std::numeric_limits<float>::infinity();
        box.max[i] = -std::numeric_limits<float>::infinity();
    }

    for (int i = 0; i < length; i++) {
        for (int j = 0; j < N; j++) {
            float value = points[i][j];
            if (value < box.min[j]) box.min[j] = value;
            if (value > box.max[j]) box.max[j] = value;
        }
    }

    return box;
}