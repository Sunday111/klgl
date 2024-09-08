#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlVertexAttributeTypeToGlValue = []
{
    using T = GlVertexAttributeType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 34);

    c.Add(T::Float, GL_FLOAT);
    c.Add(T::FloatVec2, GL_FLOAT_VEC2);
    c.Add(T::FloatVec3, GL_FLOAT_VEC3);
    c.Add(T::FloatVec4, GL_FLOAT_VEC4);
    c.Add(T::FloatMat2, GL_FLOAT_MAT2);
    c.Add(T::FloatMat3, GL_FLOAT_MAT3);
    c.Add(T::FloatMat4, GL_FLOAT_MAT4);
    c.Add(T::FloatMat2x3, GL_FLOAT_MAT2x3);
    c.Add(T::FloatMat2x4, GL_FLOAT_MAT2x4);
    c.Add(T::FloatMat3x2, GL_FLOAT_MAT3x2);
    c.Add(T::FloatMat3x4, GL_FLOAT_MAT3x4);
    c.Add(T::FloatMat4x2, GL_FLOAT_MAT4x2);
    c.Add(T::FloatMat4x3, GL_FLOAT_MAT4x3);
    c.Add(T::Int, GL_INT);
    c.Add(T::IntVec2, GL_INT_VEC2);
    c.Add(T::IntVec3, GL_INT_VEC3);
    c.Add(T::IntVec4, GL_INT_VEC4);
    c.Add(T::UnsigneInt, GL_UNSIGNED_INT);
    c.Add(T::UnsigneIntVec2, GL_UNSIGNED_INT_VEC2);
    c.Add(T::UnsigneIntVec3, GL_UNSIGNED_INT_VEC3);
    c.Add(T::UnsigneIntVec4, GL_UNSIGNED_INT_VEC4);
    c.Add(T::Double, GL_DOUBLE);
    c.Add(T::DoubleVec2, GL_DOUBLE_VEC2);
    c.Add(T::DoubleVec3, GL_DOUBLE_VEC3);
    c.Add(T::DoubleVec4, GL_DOUBLE_VEC4);
    c.Add(T::DoubleMat2, GL_DOUBLE_MAT2);
    c.Add(T::DoubleMat3, GL_DOUBLE_MAT3);
    c.Add(T::DoubleMat4, GL_DOUBLE_MAT4);
    c.Add(T::DoubleMat2x3, GL_DOUBLE_MAT2x3);
    c.Add(T::DoubleMat2x4, GL_DOUBLE_MAT2x4);
    c.Add(T::DoubleMat3x2, GL_DOUBLE_MAT3x2);
    c.Add(T::DoubleMat3x4, GL_DOUBLE_MAT3x4);
    c.Add(T::DoubleMat4x2, GL_DOUBLE_MAT4x2);
    c.Add(T::DoubleMat4x3, GL_DOUBLE_MAT4x3);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlVertexAttributeType value) noexcept
{
    return detail::kGlVertexAttributeTypeToGlValue.to_gl_value.Get(value);
}

[[nodiscard]] constexpr GlVertexAttributeType FromGlValue(GLenum value) noexcept
{
    return detail::kGlVertexAttributeTypeToGlValue.from_gl_enum.Get(value);
}

}  // namespace klgl
