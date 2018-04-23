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