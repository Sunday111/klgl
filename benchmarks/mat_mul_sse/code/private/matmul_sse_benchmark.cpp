#include <benchmark/benchmark.h>
#include <pmmintrin.h>
#include <xmmintrin.h>

#include <random>

#include "EverydayTools/Math/Matrix.hpp"

edt::Mat4f M4x4_SSE_1(const edt::Mat4f& ma, const edt::Mat4f& mb)
{
    edt::Mat4f mc;
    __m128 row1 = _mm_load_ps(&mb(0, 0));
    __m128 row2 = _mm_load_ps(&mb(1, 0));
    __m128 row3 = _mm_load_ps(&mb(2, 0));
    __m128 row4 = _mm_load_ps(&mb(3, 0));
    for (size_t i = 0; i != 4; i++)
    {
        _mm_store_ps(
            &mc(i, 0),
            _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_set1_ps(ma(i, 0)), row1), _mm_mul_ps(_mm_set1_ps(ma(i, 1)), row2)),
                _mm_add_ps(_mm_mul_ps(_mm_set1_ps(ma(i, 2)), row3), _mm_mul_ps(_mm_set1_ps(ma(i, 3)), row4))));
    }

    return mc;
}

edt::Mat4f M4x4_SSE_2(const edt::Mat4f& a, const edt::Mat4f& b)
{
    edt::Mat4f mc;
    __m128 r0 = _mm_load_ps(&b.At<0, 0>());
    __m128 r1 = _mm_load_ps(&b.At<1, 0>());
    __m128 r3 = _mm_load_ps(&b.At<2, 0>());
    __m128 r4 = _mm_load_ps(&b.At<3, 0>());

    _mm_store_ps(
        &mc.At<0, 0>(),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<0, 0>()), r0), _mm_mul_ps(_mm_set1_ps(a.At<0, 1>()), r1)),
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<0, 2>()), r3), _mm_mul_ps(_mm_set1_ps(a.At<0, 3>()), r4))));

    _mm_store_ps(
        &mc.At<1, 0>(),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<1, 0>()), r0), _mm_mul_ps(_mm_set1_ps(a.At<1, 1>()), r1)),
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<1, 2>()), r3), _mm_mul_ps(_mm_set1_ps(a.At<1, 3>()), r4))));

    _mm_store_ps(
        &mc.At<2, 0>(),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<2, 0>()), r0), _mm_mul_ps(_mm_set1_ps(a.At<2, 1>()), r1)),
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<2, 2>()), r3), _mm_mul_ps(_mm_set1_ps(a.At<2, 3>()), r4))));

    _mm_store_ps(
        &mc.At<3, 0>(),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<3, 0>()), r0), _mm_mul_ps(_mm_set1_ps(a.At<3, 1>()), r1)),
            _mm_add_ps(_mm_mul_ps(_mm_set1_ps(a.At<3, 2>()), r3), _mm_mul_ps(_mm_set1_ps(a.At<3, 3>()), r4))));

    return mc;
}

static constexpr size_t kNumIters = 1'000'000;
static constexpr double kWarmup = 0.1;

template <typename Fn>
static void MatMulGeneric(benchmark::State& state, Fn&& fn)
{
    static constexpr unsigned kSeed = 12345;
    std::mt19937 rnd(kSeed);  // NOLINT
    std::uniform_real_distribution<float> distr(-1.f, 1.f);

    std::vector<edt::Mat4f> a(kNumIters);
    std::vector<edt::Mat4f> b(kNumIters);

    for (size_t i = 0; i != kNumIters; ++i)
    {
        for (size_t j = 0; j != 16; ++j)
        {
            a[i].data_[j] = distr(rnd);
            b[i].data_[j] = distr(rnd);
        }
    }

    for (auto _ : state)
    {
        for (size_t i = 0; i != kNumIters; ++i)
        {
            auto result = fn(a[i], b[i]);
            benchmark::DoNotOptimize(result);
        }
    }
}

static void BM_SimpleMatrixMultiplication(benchmark::State& state)
{
    MatMulGeneric(state, [](const edt::Mat4f& a, const edt::Mat4f& b) { return a.MatMul(b); });
}

static void BM_MatrixMultiplicationSSE1(benchmark::State& state)
{
    MatMulGeneric(state, M4x4_SSE_1);
}

static void BM_MatrixMultiplicationSSE2(benchmark::State& state)
{
    MatMulGeneric(state, M4x4_SSE_2);
}

BENCHMARK(BM_SimpleMatrixMultiplication)->MinWarmUpTime(kWarmup);
BENCHMARK(BM_MatrixMultiplicationSSE1)->MinWarmUpTime(kWarmup);
BENCHMARK(BM_MatrixMultiplicationSSE2)->MinWarmUpTime(kWarmup);

// Run the benchmark
BENCHMARK_MAIN();  // NOLINT
