#pragma once

#include <cassert>
#include <optional>

#include "EverydayTools/Math/Math.hpp"
#include "klgl/math/rotator.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class Camera3d
{
public:
    // dir and up have to be normalized.
    [[nodiscard]] static constexpr edt::Mat4f
    LookAtRH(const edt::Vec3f& eye, const edt::Vec3f& dir, const edt::Vec3f& up) noexcept
    {
        // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_transform.inl#L153
        const auto& f = dir;
        const auto s = f.Cross(up);
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

    // dir and up have to be normalized.
    [[nodiscard]] static constexpr edt::Mat4f
    MakeOpenGLViewMatrix(const edt::Vec3f& eye, const edt::Vec3f& dir, const edt::Vec3f& up) noexcept
    {
        auto r = LookAtRH(eye, dir, up);
        r.SetColumn(0, -r.GetColumn(0));
        return r;
    }

    [[nodiscard]] static edt::Mat4f PerspectiveLH(float fovy, float aspect, float zNear, float zFar) noexcept
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

    [[nodiscard]] static edt::Mat4f PerspectiveRH(float fovy, float aspect, float zNear, float zFar) noexcept
    {
        // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_clip_space.inl#L249
        auto r = PerspectiveLH(fovy, aspect, zNear, zFar);
        r.At<2, 3>() = -1;
        return r;
    }

    constexpr Camera3d() noexcept = default;
    constexpr Camera3d(const Camera3d&) noexcept = default;
    constexpr Camera3d(Camera3d&&) noexcept = default;
    constexpr Camera3d& operator=(const Camera3d&) noexcept = default;
    constexpr Camera3d& operator=(Camera3d&&) noexcept = default;
    constexpr Camera3d(const Vec3f& eye, const Rotator& r) noexcept : rotation_(r), eye_(eye) {}

    bool Widget();

    /****************************************************** View ******************************************************/

    [[nodiscard]] const Rotator& GetRotation() const noexcept { return rotation_; }
    void SetRotation(const Rotator& rotator) noexcept
    {
        rotation_ = rotator;
        view_cache_.reset();
    }

    [[nodiscard]] const Vec3f& GetEye() const noexcept { return eye_; }
    void SetEye(const Vec3f& eye) noexcept
    {
        eye_ = eye;
        view_cache_.reset();
    }

    [[nodiscard]] const Vec3f& GetForwardAxis() const noexcept { return GetViewCache().forward; }
    [[nodiscard]] const Vec3f& GetRightAxis() const noexcept { return GetViewCache().right; }
    [[nodiscard]] const Vec3f& GetUpAxis() const noexcept { return GetViewCache().up; }

    // Returns matrix that can be used as view matrix in OpenGL shader.
    // It is already transposed so can be used as is
    [[nodiscard]] const Mat4f& GetViewMatrix() const noexcept { return GetViewCache().view_matrix; }

    /*************************************************** Projection ***************************************************/

    // Projection matrix for OpenGL.
    // Already transposed so can be used as is.
    edt::Mat4f GetProjectionMatrix(float aspect) const noexcept
    {
        return PerspectiveRH(edt::Math::DegToRad(fov_), aspect, near_, far_);
    }

    [[nodiscard]] constexpr float GetNear() const noexcept { return near_; }
    constexpr void SetNear(float near) noexcept { near_ = near; }

    [[nodiscard]] constexpr float GetFar() const noexcept { return far_; }
    constexpr void SetFar(float far) noexcept { far_ = far; }

    [[nodiscard]] constexpr float GetFOV() const noexcept { return fov_; }
    constexpr void SetFOV(float fov) noexcept { fov_ = fov; }

private:
    struct ViewCache
    {
        Vec3f forward;
        Vec3f right;
        Vec3f up;
        Mat4f rotator_matrix;
        Mat4f view_matrix;
    };

    const ViewCache& GetViewCache() const noexcept
    {
        if (!view_cache_.has_value())
        {
            view_cache_ = ViewCache{};
            auto& vc = view_cache_.value();
            vc.rotator_matrix = rotation_.ToMatrix();
            edt::Math::ToBasisVectors(vc.rotator_matrix, &vc.forward, &vc.right, &vc.up);
            vc.view_matrix = MakeOpenGLViewMatrix(eye_, vc.forward, vc.up);
        }

        return view_cache_.value();
    }

    mutable std::optional<ViewCache> view_cache_;
    float near_ = 0.1f;
    float far_ = 100.f;
    float fov_ = 45.f;
    Rotator rotation_;
    Vec3f eye_;
};
}  // namespace klgl
