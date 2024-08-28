#pragma once

#include <cassert>

#include "EverydayTools/Math/Math.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class Camera3d
{
public:
    [[nodiscard]] static constexpr edt::Mat4f
    LookAt(const edt::Vec3f& eye, const edt::Vec3f& center, const edt::Vec3f& up)
    {
        const auto f = (center - eye).Normalized();
        const auto s = f.Cross(up.Normalized()).Normalized();
        const auto u = s.Cross(f);

        edt::Mat4f Result = edt::Mat4f::Identity();
        Result(0, 0) = s.x();
        Result(1, 0) = s.y();
        Result(2, 0) = s.z();
        Result(0, 1) = u.x();
        Result(1, 1) = u.y();
        Result(2, 1) = u.z();
        Result(0, 2) = -f.x();
        Result(1, 2) = -f.y();
        Result(2, 2) = -f.z();
        Result(3, 0) = -s.Dot(eye);
        Result(3, 1) = -u.Dot(eye);
        Result(3, 2) = f.Dot(eye);
        return Result;
    }

    [[nodiscard]] static constexpr edt::Mat4f Perspective(float fovy, float aspect, float zNear, float zFar)
    {
        assert(std::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.f);

        const float tanHalfFovy = std::tan(fovy / 2.f);
        auto r = edt::Mat4f{};
        r(0, 0) = 1.f / (aspect * tanHalfFovy);
        r(1, 1) = 1.f / (tanHalfFovy);
        r(2, 2) = -(zFar + zNear) / (zFar - zNear);
        r(2, 3) = -1.f;
        r(3, 2) = -(2.f * zFar * zNear) / (zFar - zNear);
        return r;
    }

    constexpr Camera3d() noexcept { Reset(); }

    bool Widget();

    constexpr void Reset()
    {
        eye_ = {0.f, -3.f, 0.f};
        dir_ = {0.f, 1.f, 0.f};
        right_ = {1.f, 0.f, 0.f};
        fov_ = 45.f;
        near_ = 0.1f;
        far_ = 10.f;
    }

    constexpr edt::Mat4f GetProjectionMatrix(float aspect) const
    {
        return Perspective(edt::Math::DegToRad(fov_), aspect, near_, far_);
    }

    constexpr edt::Mat4f GetViewMatrix() const { return LookAt(eye_, eye_ + dir_, GetUp()); }

    constexpr edt::Vec3f GetUp() const noexcept { return -dir_.Cross(right_); }

    edt::Vec3f eye_;
    edt::Vec3f dir_;
    edt::Vec3f right_;
    float fov_ = 45;
    float near_ = 0.1f;
    float far_ = 10.f;
};
}  // namespace klgl
