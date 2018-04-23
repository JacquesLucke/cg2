#include "vector.h"
#include "random.h"

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