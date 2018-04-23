#include <iostream>
#include "bounding_box.h"

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