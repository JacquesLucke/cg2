#include "vector.h"
#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<Point<3>> *points = generateRandomPoints<3>(10);
    for (size_t i = 0; i < points->size(); i++) {
        std::cout << (*points)[i] << std::endl;
    }
    return 0;
}
