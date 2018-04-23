#pragma once

#include <iostream>
#include <array>
#include <cmath>
#include <vector>
#include "random.h"

template<int N>
struct Vector {
    std::array<float, N> data;

public:

    friend std::ostream& operator<<(std::ostream& stream, const Vector& p) {
        stream << "(";
        for (size_t i = 0; i < N; i++) {
            stream << p.data[i];
            if (i < N - 1) stream << ", ";
        }
        stream << ")";
        return stream;
    }

    float& operator[](const int i) {
        return this->data[i];
    }

    static float distanceSquared(Vector &a, Vector &b) {
        float sum = 0;
        for (int i = 0; i < N; i++) {
            float value = a.data[i] - b.data[i];
            sum += value * value;
        }
        return sum;
    }

    static float distance(Vector &a, Vector &b) {
        return sqrt(distanceSquared(a, b));
    }
};

template<int N>
inline float getVectorIndex(Vector<N> &vector, int axis) {
    return vector[axis];
}

template<int N>
std::vector<Vector<N>> generateRandomVectors(int amount, int seed) {
    std::vector<Vector<N>> points;
    for (int i = 0; i < amount; i++) {
        auto point = Vector<N>();
        for (int j = 0; j < N; j++) {
            point[j] = randomFloat_Range(seed, 10);
            seed += 5233;
        }
        points.push_back(point);
    }

    return points;
}

template<int N>
void printVectors(Vector<N> *vectors, int length) {
    for (int i = 0; i < length; i++) {
        std::cout << vectors[i] << std::endl;
    }
}

template<int N>
void printVectors(std::vector<Vector<N>> &vectors) {
    printVectors(vectors.data(), vectors.size());
}