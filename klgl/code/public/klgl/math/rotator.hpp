#pragma once

#include "EverydayTools/Math/Math.hpp"

namespace klgl
{

// This struct generatres transformation matrix
// with 3 rotations applied in fixed order: yaw -> pitch -> roll
class Rotator
{
    [[nodiscard]] static constexpr edt::Mat4f ToMatrixXY(const Rotator&) noexcept { return edt::Mat4f::Identity(); }

    static constexpr edt::Mat4f ImplIdentity(const Rotator&) { return edt::Mat4f::Identity(); }
    static constexpr edt::Mat4f ImplX(const Rotator& r)
    {
        return edt::Math::RotationMatrix3dX(edt::Math::DegToRad(r.roll));
    }

    static constexpr edt::Mat4f ImplY(const Rotator& r)
    {
        return edt::Math::RotationMatrix3dY(edt::Math::DegToRad(r.pitch));
    }

    static constexpr edt::Mat4f ImplZ(const Rotator& r)
    {
        return edt::Math::RotationMatrix3dZ(edt::Math::DegToRad(r.yaw));
    }

    static constexpr edt::Mat4f ImplXY(const Rotator& r) { return ImplY(r).MatMul(ImplX(r)); }
    static constexpr edt::Mat4f ImplXZ(const Rotator& r) { return ImplZ(r).MatMul(ImplX(r)); }
    static constexpr edt::Mat4f ImplYZ(const Rotator& r) { return ImplZ(r).MatMul(ImplY(r)); }
    static constexpr edt::Mat4f ImplXYZ(const Rotator& r) { return ImplYZ(r).MatMul(ImplX(r)); }

public:
    [[nodiscard]] constexpr edt::Mat4f ToMatrix() const noexcept
    {
        // This is an inlined varsion of multiplication between three matrices
        // https://en.wikipedia.org/wiki/Rotation_matrix#General_3D_rotations

        float sa, ca, sb, cb, sg, cg;  // NOLINT
        edt::Math::SinCos(edt::Math::DegToRad(roll), sa, ca);
        edt::Math::SinCos(edt::Math::DegToRad(pitch), sb, cb);
        edt::Math::SinCos(edt::Math::DegToRad(yaw), sg, cg);

        edt::Mat4f m;

        m.At<0, 0>() = cb * cg;
        m.At<0, 1>() = sa * sb * cg - ca * sg;
        m.At<0, 2>() = ca * sb * cg + sa * sg;

        m.At<1, 0>() = cb * sg;
        m.At<1, 1>() = sa * sb * sg + ca * cg;
        m.At<1, 2>() = ca * sb * sg - sa * cg;

        m.At<2, 0>() = -sb;
        m.At<2, 1>() = sa * cb;
        m.At<2, 2>() = ca * cb;

        m.At<3, 0>() = 0.f;
        m.At<3, 1>() = 0.f;
        m.At<3, 2>() = 0.f;

        m.At<3, 3>() = 1.f;

        return m;
    }

    // Rotation around Z (Up) axis, degrees
    float yaw = 0.f;

    // Rotation around Y (Right) axis, degrees
    float pitch = 0.f;

    // Rotation around X (Forward) axis, degrees
    float roll = 0.f;
};
}  // namespace klgl
