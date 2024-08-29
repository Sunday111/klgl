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
    [[nodiscard]] constexpr edt::Mat4f ToMatrix() const noexcept { return ImplXYZ(*this); }

    // Rotation around Z (Up) axis, degrees
    float yaw = 0.f;

    // Rotation around Y (Right) axis, degrees
    float pitch = 0.f;

    // Rotation around X (Forward) axis, degrees
    float roll = 0.f;
};
}  // namespace klgl
