#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureWrapAxisToGlValue = []
{
    using T = GlTextureWrapAxis;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 3);
    c.Add(T::S, GL_TEXTURE_WRAP_S);
    c.Add(T::R, GL_TEXTURE_WRAP_R);
    c.Add(T::T, GL_TEXTURE_WRAP_T);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlTextureWrapAxis axis) noexcept
{
    return detail::kGlTextureWrapAxisToGlValue.to_gl_value.Get(axis);
}

}  // namespace klgl
