#pragma once

#include <thread>

inline int getMedianIndex(int left, int right) {
    return (left + right) / 2;
}

int getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}

#ifdef __unix__
    std::string SLASH = "/";
#else
    std::string SLASH = "\\";
#endif