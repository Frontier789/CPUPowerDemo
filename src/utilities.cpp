#include "utilities.h"

#include <random>

std::vector<float> generateMatrix(size_t N)
{
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist(0, 1);
    std::vector<float> mat(N * N);

    for (auto &f : mat)
    {
        f = dist(rng);
    }

    return mat;
}

bool matricesMatch(const std::vector<float> &left, const std::vector<float> &right, size_t N)
{
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (left[i * N + j] != right[i * N + j])
                return false;

    return true;
}