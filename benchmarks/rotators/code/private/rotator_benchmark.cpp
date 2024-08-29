#include <benchmark/benchmark.h>

#include <random>
#include <vector>

#include "klgl/math/rotator.hpp"

edt::Mat4f MatMul3(float yaw, float pitch, float roll)
{
    float sa, ca, sb, cb, sg, cg;  // NOLINT
    edt::Math::SinCos(edt::Math::DegToRad(roll), sa, ca);
    edt::Math::SinCos(edt::Math::DegToRad(pitch), sb, cb);
    edt::Math::SinCos(edt::Math::DegToRad(yaw), sg, cg);

    edt::Mat4f m;
    m(0, 0) = cb * cg;
    m(0, 1) = sa * sb * cg - ca * sg;
    m(0, 2) = ca * sb * cg + sa * sg;

    m(1, 0) = cb * sg;
    m(1, 1) = sa * sb * sg + ca * cg;
    m(1, 2) = ca * sb * sg - sa * cg;

    m(2, 0) = -sb;
    m(2, 1) = sa * cb;
    m(2, 2) = ca * cb;

    m(3, 0) = 0.f;
    m(3, 1) = 0.f;
    m(3, 2) = 0.f;
    m(3, 3) = 1.f;

    return m;
}

static std::vector<float> MakeRandomVector(const size_t n)
{
    std::mt19937 gen(0);  // NOLINT
    std::vector<float> values;
    values.reserve(n);
    std::uniform_real_distribution<float> distr(-180.f, 180.f);
    for (size_t i = 0; i != n; ++i)
    {
        values.push_back(distr(gen));
    }

    return values;
}

constexpr size_t kNumIterations = 1000000;

template <size_t P>
static void BM_Manual(benchmark::State& state)
{
    const auto values = MakeRandomVector(kNumIterations * P);

    edt::Mat4f r{};

    for (auto _ : state)
    {
        for (size_t i = 0; i != kNumIterations; ++i)
        {
            float yaw{}, pitch{}, roll{};

            if constexpr (P > 0) yaw = values[i * P + 0];
            if constexpr (P > 1) pitch = values[i * P + 1];
            if constexpr (P > 2) roll = values[i * P + 2];

            r += MatMul3(yaw, pitch, roll);
        }
    }

    benchmark::DoNotOptimize(r);
}

template <size_t P>
static void BM_Rotator(benchmark::State& state)
{
    const auto values = MakeRandomVector(kNumIterations * P);
    edt::Mat4f r{};

    for (auto _ : state)
    {
        for (size_t i = 0; i != kNumIterations; ++i)
        {
            float yaw{}, pitch{}, roll{};

            if constexpr (P > 0) yaw = values[i * P + 0];
            if constexpr (P > 1) pitch = values[i * P + 1];
            if constexpr (P > 2) roll = values[i * P + 2];

            r += klgl::Rotator{.yaw = yaw, .pitch = pitch, .roll = roll}.ToMatrix();
        }
    }

    benchmark::DoNotOptimize(r);
}

static void BM_Manual_0(benchmark::State& state)
{
    BM_Manual<0>(state);
}

static void BM_Manual_1(benchmark::State& state)
{
    BM_Manual<1>(state);
}

static void BM_Manual_2(benchmark::State& state)
{
    BM_Manual<2>(state);
}

static void BM_Manual_3(benchmark::State& state)
{
    BM_Manual<3>(state);
}

static void BM_Rotator_0(benchmark::State& state)
{
    BM_Rotator<0>(state);
}

static void BM_Rotator_1(benchmark::State& state)
{
    BM_Rotator<1>(state);
}

static void BM_Rotator_2(benchmark::State& state)
{
    BM_Rotator<2>(state);
}

static void BM_Rotator_3(benchmark::State& state)
{
    BM_Rotator<3>(state);
}

BENCHMARK(BM_Rotator_0);
BENCHMARK(BM_Rotator_1);
BENCHMARK(BM_Rotator_2);
BENCHMARK(BM_Rotator_3);
BENCHMARK(BM_Manual_0);
BENCHMARK(BM_Manual_1);
BENCHMARK(BM_Manual_2);
BENCHMARK(BM_Manual_3);

// Run the benchmark
BENCHMARK_MAIN();  // NOLINT
