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
    LookAtLH(const edt::Vec3f& eye, const edt::Vec3f& dir, const edt::Vec3f& up) noexcept
    {
        // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_transform.inl#L176
        const auto& f = dir;
        const auto s = up.Cross(f).Normalized();
        const auto u = f.Cross(s);

        Mat4f r = Mat4f::Identity();
        r.At<0, 0>() = s.x();
        r.At<1, 0>() = s.y();
        r.At<2, 0>() = s.z();
        r.At<0, 1>() = u.x();
        r.At<1, 1>() = u.y();
        r.At<2, 1>() = u.z();
        r.At<0, 2>() = f.x();
        r.At<1, 2>() = f.y();
        r.At<2, 2>() = f.z();
        r.At<3, 0>() = -s.Dot(eye);
        r.At<3, 1>() = -u.Dot(eye);
        r.At<3, 2>() = -f.Dot(eye);
        return r;
    }

    [[nodiscard]] static constexpr edt::Mat4f
    LookAtRH(const edt::Vec3f& eye, const edt::Vec3f& dir, const edt::Vec3f& up) noexcept
    {
        // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_transform.inl#L153
        const auto& f = dir;
        const auto s = f.Cross(up).Normalized();
        const auto u = s.Cross(f);
        auto r = Mat4f::Identity();
        r.At<0, 0>() = s.x();
        r.At<1, 0>() = s.y();
        r.At<2, 0>() = s.z();
        r.At<0, 1>() = u.x();
        r.At<1, 1>() = u.y();
        r.At<2, 1>() = u.z();
        r.At<0, 2>() = -f.x();
        r.At<1, 2>() = -f.y();
        r.At<2, 2>() = -f.z();
        r.At<3, 0>() = -s.Dot(eye);
        r.At<3, 1>() = -u.Dot(eye);
        r.At<3, 2>() = f.Dot(eye);
        return r;
    }

    [[nodiscard]] static constexpr edt::Mat4f PerspectiveLH(float fovy, float aspect, float zNear, float zFar) noexcept
    {
        // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_clip_space.inl#L281
        assert(std::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.f);
        const float tanHalfFovy = std::tan(fovy / 2.f);
        auto r = edt::Mat4f{};
        r.At<0, 0>() = 1.f / (aspect * tanHalfFovy);
        r.At<1, 1>() = 1.f / (tanHalfFovy);
        r.At<2, 2>() = -(zFar + zNear) / (zFar - zNear);
        r.At<2, 3>() = 1.f;
        r.At<3, 2>() = -(2.f * zFar * zNear) / (zFar - zNear);
        return r;
    }

    [[nodiscard]] static constexpr edt::Mat4f PerspectiveRH(float fovy, float aspect, float zNear, float zFar) noexcept
    {
        // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_clip_space.inl#L249
        auto r = PerspectiveLH(fovy, aspect, zNear, zFar);
        r.At<2, 3>() = -1;
        return r;
    }

    bool Widget();

    constexpr edt::Mat4f GetProjectionMatrix(float aspect) const noexcept
    {
        return PerspectiveRH(edt::Math::DegToRad(fov_), aspect, near_, far_);
    }

    constexpr edt::Mat4f GetViewMatrix() const { return LookAtRH(eye_, dir_, GetUp()); }

    constexpr edt::Vec3f GetUp() const noexcept { return dir_.Cross(-right_); }

    edt::Vec3f eye_ = {0.f, 3.f, 0.f};
    edt::Vec3f dir_ = {0.f, -1.f, 0.f};
    edt::Vec3f right_ = {-1.f, 0.f, 0.f};
    float fov_ = 45;
    float near_ = 0.1f;
    float far_ = 10.f;
};
}  // namespace klgl
