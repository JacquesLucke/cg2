#include "vector.h"
#include <stdint.h>

uint32_t randomInteger(uint32_t x) {
    x = (x << 13) ^ x;
    return x * (x * x * 15731 + 789221) + 1376312589;
}

float randomFloat(uint32_t x) {
    return (randomInteger(x) & 0x7fffffff) / 2147483648.0;
}

template<int N>
std::vector<Point<N>> *generateRandomPoints(size_t amount) {
    auto *points = new std::vector<Point<N>>();
    for (size_t i = 0; i < amount*N; i += N) {
        Point<N> point = Point<N>();
        for (size_t j = 0; j < N; j++) {
            point[j] = randomFloat(i + j);
        }
        points->push_back(point);
    }
    return points;
}
