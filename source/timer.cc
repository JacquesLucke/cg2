#include <chrono>
#include <iostream>
#include "timer.h"

Timer::Timer(const char *name) {
    this->name = name;
    this->start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    double ms = duration.count() * 1000.0f;
    std::cout << "Timer '" << name << "' took " << ms << " ms" << std::endl;
}