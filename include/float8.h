#pragma once

#include "utilities.h"
#include <x86intrin.h>

typedef float float8_t __attribute__((vector_size(8 * sizeof(float))));

constexpr float8_t inf8{inf, inf, inf, inf, inf, inf, inf, inf};

inline float hmin8(float8_t values)
{
    float v = inf;
    for (int i = 0; i < 8; ++i)
    {
        v = std::min(values[i], v);
    }
    return v;
}

inline float8_t min8(float8_t x, float8_t y)
{
    return x < y ? x : y;
}

inline float8_t swap4(float8_t x)
{
    return _mm256_permute2f128_ps(x, x, 0b00000001);
}

inline float8_t swap2(float8_t x)
{
    return _mm256_permute_ps(x, 0b01001110);
}

inline float8_t swap1(float8_t x)
{
    return _mm256_permute_ps(x, 0b10110001);
}
