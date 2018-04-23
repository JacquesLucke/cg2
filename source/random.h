#pragma once

inline int randomInt(int x) {
    x = (x<<13) ^ x;
    return x * (x * x * 15731 + 789221) + 1376312589;
}

inline int randomInt_Positive(int x) {
    return randomInt(x) & 0x7fffffff;
}

inline float randomFloat_Range(int x, float scale) {
    return randomInt(x) / 2147483648.0f * scale;
}