#pragma once

#include <chrono>
#include <iostream>

class Timer {
    const char *name;
    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<float> duration;

public:
    Timer(const char *name = "");
    ~Timer();
};

#define TIMEIT(name) Timer t(name);