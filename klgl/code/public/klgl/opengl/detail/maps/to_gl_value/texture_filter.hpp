#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureFilterToGlValue = []
{
    using T = GlTextureFilter;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 6);
    c.Add(T::Nearest, GL_NEAREST);
    c.Add(T::Linear, GL_LINEAR);
    c.Add(T::NearestMipmapNearest, GL_NEAREST_MIPMAP_NEAREST);
    c.Add(T::LinearMipmapNearest, GL_LINEAR_MIPMAP_NEAREST);
    c.Add(T::NearestMipmapLinear, GL_NEAREST_MIPMAP_LINEAR);
    c.Add(T::LinearMipmapLinear, GL_LINEAR_MIPMAP_LINEAR);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLint ToGlValue(GlTextureFilter filter) noexcept
{
    return detail::kGlTextureFilterToGlValue.to_gl_value.Get(filter);
}

}  // namespace klgl
