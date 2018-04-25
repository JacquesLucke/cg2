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

std::string readFile(std::string path) {
    std::ifstream fs(path);
    std::stringstream buffer;
    buffer << fs.rdbuf();
    fs.close();
    return buffer.str();
}
