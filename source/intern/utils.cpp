#include <thread>
#include <fstream>
#include <sstream>
#include "../utils.hpp"

inline int getMedianIndex(int left, int right) {
    return (left + right) / 2;
}

int getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}