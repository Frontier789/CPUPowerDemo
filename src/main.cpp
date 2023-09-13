#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "float8.h"
#include "measure.h"
#include "utilities.h"

std::vector<float> shortcut_0(const std::vector<float> &distances, size_t N)
{
    std::vector<float> mat(N * N);

    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
        {
            float minimum = inf;

            for (size_t k = 0; k < N; ++k)
            {
                __asm("//V0: k loop begin");

                const auto x = distances[i * N + k];
                const auto y = distances[k * N + j];
                const auto z = x + y;

                minimum = std::min(z, minimum);
            }

            mat[i * N + j] = minimum;
        }

    return mat;
}

std::vector<float> shortcut_1(const std::vector<float> &distances, size_t N)
{
    std::vector<float> mat(N * N);

    parallelFor(N, [&](size_t i) {
        for (size_t j = 0; j < N; ++j)
        {
            float minimum = inf;

            for (size_t k = 0; k < N; ++k)
            {
                __asm("//V1: k loop begin");

                const auto x = distances[i * N + k];
                const auto y = distances[k * N + j];
                const auto z = x + y;

                minimum = std::min(z, minimum);
            }

            mat[i * N + j] = minimum;
        }
    });

    return mat;
}

std::vector<float> shortcut_2(const std::vector<float> &distances, size_t N)
{
    std::vector<float> distances_T(N * N);
    std::vector<float> mat(N * N);

    parallelFor(N, [&](size_t i) {
        for (size_t j = 0; j < N; ++j)
        {
            distances_T[i * N + j] = distances[j * N + i];
        }
    });

    parallelFor(N, [&](size_t i) {
        for (size_t j = 0; j < N; ++j)
        {
            float minimum = inf;

            for (size_t k = 0; k < N; ++k)
            {
                __asm("//V2: k loop begin");

                const auto x = distances[i * N + k];
                const auto y = distances_T[j * N + k];
                const auto z = x + y;

                minimum = std::min(z, minimum);
            }

            mat[i * N + j] = minimum;
        }
    });

    return mat;
}

std::vector<float> shortcut_3(const std::vector<float> &distances, size_t N)
{
    constexpr size_t KSTEP = 4;
    const auto N_pad = nextMultipleOf(N, KSTEP);

    std::vector<float> distances_pad(N * N_pad);
    std::vector<float> distances_pad_T(N * N_pad);
    std::vector<float> mat(N * N);

    parallelFor(N, [&](size_t i) {
        for (size_t j = 0; j < N; ++j)
        {
            distances_pad[i * N_pad + j] = distances[i * N + j];
            distances_pad_T[i * N_pad + j] = distances[j * N + i];
        }
    });

    parallelFor(N, [&](size_t i) {
        for (size_t j = 0; j < N; ++j)
        {
            float mins[KSTEP];
            std::fill_n(mins, KSTEP, inf);

            for (size_t ko = 0; ko < N; ko += KSTEP)
            {
                __asm("//V3: k loop begin");

                for (size_t ki = 0; ki < KSTEP; ++ki)
                {
                    const size_t k = ko + ki;

                    const auto x = distances_pad[i * N_pad + k];
                    const auto y = distances_pad_T[j * N_pad + k];
                    const auto z = x + y;

                    mins[ki] = std::min(z, mins[ki]);
                }
            }

            float min = inf;
            for (float m : mins)
                min = std::min(min, m);

            mat[i * N + j] = min;
        }
    });

    return mat;
}

std::vector<float> shortcut_4(const std::vector<float> &distances, size_t N)
{
    constexpr size_t VECD = 8;
    const auto N_vec = nextMultipleOf(N, VECD) / VECD;

    std::vector<float8_t> distances_vec(N * N_vec);
    std::vector<float8_t> distances_vec_T(N * N_vec);
    std::vector<float> mat(N * N);

    parallelFor(N, [&](size_t i) {
        for (size_t jo = 0; jo < N_vec; ++jo)
        {
            for (size_t ji = 0; ji < VECD; ++ji)
            {
                size_t j = jo * VECD + ji;

                distances_vec[i * N_vec + jo][ji] = j < N ? distances[i * N + j] : inf;
                distances_vec_T[i * N_vec + jo][ji] = j < N ? distances[j * N + i] : inf;
            }
        }
    });

    parallelFor(N, [&](size_t i) {
        for (size_t j = 0; j < N; ++j)
        {
            float8_t min = inf8;

            for (size_t k = 0; k < N_vec; ++k)
            {
                __asm("//V4: k loop begin");

                const auto x = distances_vec[i * N_vec + k];
                const auto y = distances_vec_T[j * N_vec + k];
                const auto z = x + y;

                min = min8(z, min);
            }

            mat[i * N + j] = hmin8(min);
        }
    });

    return mat;
}

std::vector<float> shortcut_5(const std::vector<float> &distances, size_t N)
{
    constexpr size_t VECD = 8;
    const auto N_vec = nextMultipleOf(N, VECD) / VECD;

    constexpr size_t BLOCS = 3;
    const auto N_bloc = nextMultipleOf(N, BLOCS);

    std::vector<float8_t> distances_vec(N_bloc * N_vec);
    std::vector<float8_t> distances_vec_T(N_bloc * N_vec);
    std::vector<float> mat(N * N);

    parallelFor(N, [&](size_t i) {
        for (size_t jo = 0; jo < N_vec; ++jo)
        {
            for (size_t ji = 0; ji < VECD; ++ji)
            {
                size_t j = jo * VECD + ji;

                distances_vec[i * N_vec + jo][ji] = j < N ? distances[i * N + j] : inf;
                distances_vec_T[i * N_vec + jo][ji] = j < N ? distances[j * N + i] : inf;
            }
        }
    });

    for (size_t i = N; i < N_bloc; ++i)
    {
        for (size_t j = 0; j < N_vec; ++j)
        {
            distances_vec[i * N_vec + j] = inf8;
            distances_vec_T[i * N_vec + j] = inf8;
        }
    }

    parallelFor(N_bloc / BLOCS, [&](size_t io) {
        for (size_t jo = 0; jo < N_bloc / BLOCS; ++jo)
        {
            float8_t mins[BLOCS * BLOCS];
            std::fill_n(mins, BLOCS * BLOCS, inf8);

            for (size_t k = 0; k < N_vec; ++k)
            {
                __asm("//V5: k loop begin");

                float8_t xs[BLOCS];
                float8_t ys[BLOCS];

                for (size_t ii = 0; ii < BLOCS; ++ii)
                    xs[ii] = distances_vec[(io * BLOCS + ii) * N_vec + k];

                for (size_t ji = 0; ji < BLOCS; ++ji)
                    ys[ji] = distances_vec_T[(jo * BLOCS + ji) * N_vec + k];

                for (size_t ii = 0; ii < BLOCS; ++ii)
                {
                    for (size_t ji = 0; ji < BLOCS; ++ji)
                    {
                        size_t mini = ii * BLOCS + ji;

                        mins[mini] = min8(mins[mini], xs[ii] + ys[ji]);
                    }
                }
            }

            for (size_t ii = 0; ii < BLOCS; ++ii)
            {
                for (size_t ji = 0; ji < BLOCS; ++ji)
                {
                    size_t i = io * BLOCS + ii;
                    size_t j = jo * BLOCS + ji;

                    if (i < N && j < N)
                    {
                        mat[i * N + j] = hmin8(mins[ii * BLOCS + ji]);
                    }
                }
            }
        }
    });

    return mat;
}

std::vector<float> shortcut_6(const std::vector<float> &distances, size_t N)
{
    constexpr size_t VECD = 8;
    const auto N_vec = nextMultipleOf(N, VECD) / VECD;

    std::vector<float8_t> distances_vec(N * N_vec);
    std::vector<float8_t> distances_vec_T(N * N_vec);
    std::vector<float> mat(N * N);

    parallelFor(N_vec, [&](size_t io) {
        for (size_t ii = 0; ii < VECD; ++ii)
        {
            size_t i = io * VECD + ii;

            for (size_t j = 0; j < N; ++j)
            {
                distances_vec[io * N + j][ii] = i < N ? distances[i * N + j] : inf;
                distances_vec_T[io * N + j][ii] = i < N ? distances[j * N + i] : inf;
            }
        }
    });

    parallelFor(N_vec, [&](size_t io) {
        for (size_t jo = 0; jo < N_vec; ++jo)
        {
            float8_t vv000 = inf8;
            float8_t vv001 = inf8;
            float8_t vv010 = inf8;
            float8_t vv011 = inf8;
            float8_t vv100 = inf8;
            float8_t vv101 = inf8;
            float8_t vv110 = inf8;
            float8_t vv111 = inf8;

            for (size_t k = 0; k < N; ++k)
            {
                __asm("//V6: k loop begin");

                float8_t a000 = distances_vec[io * N + k];
                float8_t b000 = distances_vec_T[jo * N + k];

                float8_t a100 = swap4(a000);
                float8_t a010 = swap2(a000);
                float8_t a110 = swap2(a100);
                float8_t b001 = swap1(b000);

                vv000 = min8(vv000, a000 + b000);
                vv001 = min8(vv001, a000 + b001);
                vv010 = min8(vv010, a010 + b000);
                vv011 = min8(vv011, a010 + b001);
                vv100 = min8(vv100, a100 + b000);
                vv101 = min8(vv101, a100 + b001);
                vv110 = min8(vv110, a110 + b000);
                vv111 = min8(vv111, a110 + b001);
            }

            float8_t vv[8] = {vv000, swap1(vv001), vv010, swap1(vv011), vv100, swap1(vv101), vv110, swap1(vv111)};

            for (size_t ii = 0; ii < VECD; ++ii)
            {
                for (size_t ji = 0; ji < VECD; ++ji)
                {
                    size_t i = io * VECD + ii;
                    size_t j = jo * VECD + ji;

                    if (i < N && j < N)
                    {
                        mat[i * N + j] = vv[ii ^ ji][ji];
                    }
                }
            }
        }
    });

    return mat;
}

int main()
{
    bool validate = false;

    auto shortcut_fun = shortcut_6;
    auto shortcutBaseline = shortcut_5;

    for (size_t N : {100, 160, 250, 400, 650, 1000, 1600, 2500, 4000, 6300, 9500, 11400})
    {
        const auto distances = generateMatrix(N);

        if (validate)
        {
            const auto testRun = shortcut_fun(distances, N);
            const auto baselineResult = shortcutBaseline(distances, N);
            if (!matricesMatch(baselineResult, testRun, N))
            {
                std::cout << "Outputs don't match" << std::endl;
                return 1;
            }
        }

        const auto nanoseconds = measure(shortcut_fun, distances, N, 1);

        const auto operations = N * N * N * 2;

        // ops / nanosec = gops / sec
        const auto gflops = static_cast<long double>(operations) / static_cast<long double>(nanoseconds);

        std::cout << "N=" << N << ", GFLOPS=" << gflops << ", time=" << nanoseconds / 1'000'000.0L << std::endl;
    }
}
