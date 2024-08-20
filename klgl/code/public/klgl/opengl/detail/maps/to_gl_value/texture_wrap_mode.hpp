#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureWrapModeToGlValue = []
{
    using T = GlTextureWrapMode;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 5);
    c.Add(T::ClampToEdge, GL_CLAMP_TO_EDGE);
    c.Add(T::ClampToBorder, GL_CLAMP_TO_BORDER);
    c.Add(T::MirroredRepeat, GL_MIRRORED_REPEAT);
    c.Add(T::Repeat, GL_REPEAT);
    c.Add(T::MirrorClampToEdge, GL_MIRROR_CLAMP_TO_EDGE);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlTextureWrapMode wrap_mode) noexcept
{
    return detail::kGlTextureWrapModeToGlValue.to_gl_value.Get(wrap_mode);
}

}  // namespace klgl
