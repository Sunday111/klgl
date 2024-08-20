#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlVertexAttribComponentTypeToGlValue = []
{
    using T = GlVertexAttribComponentType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 13);
    c.Add(T::Byte, GL_BYTE);
    c.Add(T::UnsignedByte, GL_UNSIGNED_BYTE);
    c.Add(T::Short, GL_SHORT);
    c.Add(T::UnsignedShort, GL_UNSIGNED_SHORT);
    c.Add(T::Int, GL_INT);
    c.Add(T::UnsignedInt, GL_UNSIGNED_INT);
    c.Add(T::HalfFloat, GL_HALF_FLOAT);
    c.Add(T::Float, GL_FLOAT);
    c.Add(T::Double, GL_DOUBLE);
    c.Add(T::Fixed, GL_FIXED);
    c.Add(T::Int_2_10_10_10_Rev, GL_INT_2_10_10_10_REV);
    c.Add(T::UnsignedInt_2_10_10_10_Rev, GL_UNSIGNED_INT_2_10_10_10_REV);
    c.Add(T::UnsignedInt_10F_11F_11F_Rev, GL_UNSIGNED_INT_10F_11F_11F_REV);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlVertexAttribComponentType buffer_type) noexcept
{
    return detail::kGlVertexAttribComponentTypeToGlValue.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
