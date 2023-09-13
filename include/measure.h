#pragma once

#include "utilities.h"

template <class F>
int64_t measure(F shortcut_fun, const std::vector<float> &distances, size_t N, size_t runs = 1)
{
    int64_t fastest = 0;

    for (size_t i = 0; i < runs; ++i)
    {
        using namespace std::chrono;

        const auto start = steady_clock::now();

        shortcut_fun(distances, N);

        const auto end = steady_clock::now();
        const auto duration = duration_cast<nanoseconds>(end - start).count();

        if (i == 0 || duration < fastest)
        {
            fastest = duration;
        }
    }

    return fastest;
}