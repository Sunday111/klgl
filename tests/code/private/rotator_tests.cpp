
#include <fmt/core.h>

#include <random>

#include "gtest/gtest.h"
#include "klgl/math/rotator.hpp"

namespace klgl
{

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

        auto actual = klgl::Rotator{.yaw = yaw, .pitch = pitch, .roll = roll}.ToMatrix();
        auto expected = edt::Math::RotationMatrix3dZ(edt::Math::DegToRad(yaw))
                            .MatMul(edt::Math::RotationMatrix3dY(edt::Math::DegToRad(pitch))
                                        .MatMul(edt::Math::RotationMatrix3dX(edt::Math::DegToRad(roll))));

        for (size_t j = 0; j != 16; ++j)
        {
            ASSERT_NEAR(expected.data_[j], actual.data_[j], 0.0001f)
                << fmt::format("i: {}, j: {}, yaw: {}, pitch: {}, roll: {}", i, j, yaw, pitch, roll);
        }
    }
}
}  // namespace klgl
