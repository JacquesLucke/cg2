#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <array>

template<int N>
struct Point {
    std::array<float, N> data;

public:
    friend std::ostream& operator<<(std::ostream& stream, const Point& p) {
        stream << "P(";
        for (size_t i = 0; i < N; i++) {
            stream << p.data[i];
            if (i < N - 1) stream << ", ";
        }
        stream << ")";
        return stream;
    }

    float& operator[](const ssize_t i) {
        return this->data[i];
    }
};

struct Vector {
    float x, y, z;

    Vector(float x, float y, float z) : x(x), y(y), z(z) {}

public:
    friend std::ostream& operator<<(std::ostream& stream, const Vector& v) {
        return stream << "V(" << v.x << ", " << v.y << ", " << v.z << ")";
    }

    static float distanceSquared(Vector& v1, Vector& v2) {
        float xDiff = v1.x - v2.x;
        float yDiff = v1.y - v2.y;
        float zDiff = v1.z - v2.z;
        return xDiff * xDiff + yDiff * yDiff + zDiff * zDiff;
    }

    static float distance(Vector& v1, Vector& v2) {
        return sqrt(distanceSquared(v1, v2));
    }
};

template<int N>
std::vector<Point<N>> *generateRandomPoints(size_t amount);
