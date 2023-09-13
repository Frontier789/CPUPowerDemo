#pragma once

#include <thread>
#include <vector>

constexpr auto inf = std::numeric_limits<float>::infinity();

constexpr size_t nextMultipleOf(size_t n, size_t m)
{
    return ((n + m - 1) / m) * m;
}

std::vector<float> generateMatrix(size_t N);

bool matricesMatch(const std::vector<float> &left, const std::vector<float> &right, size_t N);

template <typename T>
void doNotOptimizeAway(const T &value)
{
    asm volatile("" ::"m"(value) : "memory");
}

// Calls a function on [0, iterations) with parallelization using std::thread
// No access control or check is imposed, it's up to the user
// to ensure no race conditions are introduced
template <typename F>
void parallelFor(size_t iterations, size_t threadCount, F f)
{
    const auto totalIterations = iterations;
    const auto iterationsPerThread = (totalIterations + threadCount - 1) / threadCount;

    auto workerRunner = [&f, iterationsPerThread, totalIterations](size_t begin) {
        const auto end = std::min(begin + iterationsPerThread, totalIterations);

        for (size_t i = begin; i < end; ++i)
        {
            f(i);
        }
    };

    std::vector<std::thread> workerThreads;
    for (size_t i = 0; i < threadCount; ++i)
    {
        auto workerBegin = i * iterationsPerThread;
        workerThreads.emplace_back(workerRunner, workerBegin); // Threads auto-start here
    }

    for (auto &t : workerThreads)
    {
        t.join();
    }
}

template <typename F>
void parallelFor(size_t iterations, F f)
{
    const size_t threadCount = std::thread::hardware_concurrency();

    parallelFor(iterations, threadCount, f);
}