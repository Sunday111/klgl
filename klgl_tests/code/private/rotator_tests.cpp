
#include <fmt/core.h>

#include <random>

#include "gtest/gtest.h"
#include "klgl/math/rotator.hpp"

namespace klgl
{

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

TEST(RotatorTest, Fuzzy)
{
    static constexpr size_t kNumIterations = 1000;
    std::mt19937 gen(0);  // NOLINT
    std::vector<float> values;
    values.reserve(1000);
    std::uniform_real_distribution<float> distr(-180.f, 180.f);
    for (size_t i = 0; i != kNumIterations; ++i)
    {
        values.push_back(distr(gen));
    }

    for (size_t i = 0; i != kNumIterations; ++i)
    {
        const float yaw = values[i * 3 + 0];

        const float pitch = values[i * 3 + 1];
        // const float pitch = 0.f;

        const float roll = values[i * 3 + 2];
        // const float roll = 0.f;

        [[maybe_unused]] auto expected = klgl::Rotator{.yaw = yaw, .pitch = pitch, .roll = roll}.ToMatrix();
        [[maybe_unused]] auto actual = MatMul3(yaw, pitch, roll);

        for (size_t j = 0; j != 16; ++j)
        {
            ASSERT_NEAR(expected.data_[j], actual.data_[j], 0.0001f)
                << fmt::format("i: {}, j: {}, yaw: {}, pitch: {}, roll: {}", i, j, yaw, pitch, roll);
        }
    }
}
}  // namespace klgl
