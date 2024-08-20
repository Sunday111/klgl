#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"
#include "opengl_value_converter.hpp"

namespace klgl::detail
{
inline constexpr auto kGlIndexBufferElementTypeConverter = []
{
    using T = GlIndexBufferElementType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 3);
    c.Add(T::UnsignedByte, GL_UNSIGNED_BYTE);
    c.Add(T::UnsignedShort, GL_UNSIGNED_SHORT);
    c.Add(T::UnsignedInt, GL_UNSIGNED_INT);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlIndexBufferElementType buffer_type) noexcept
{
    return detail::kGlIndexBufferElementTypeConverter.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
