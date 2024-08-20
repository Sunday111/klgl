#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{

inline constexpr auto kGlPixelBufferLayoutToGlValue = []
{
    using T = GlPixelBufferLayout;
    OpenGlValueConverter<T, GLint> c;

    KLGL_ENSURE_ENUM_SIZE(T, 14);
    c.Add(T::R, GL_RED);
    c.Add(T::R_Int, GL_RED_INTEGER);
    c.Add(T::Depth, GL_DEPTH_COMPONENT);
    c.Add(T::RG, GL_RG);
    c.Add(T::RG_Int, GL_RG_INTEGER);
    c.Add(T::RGB, GL_RGB);
    c.Add(T::RGB_Int, GL_RGB_INTEGER);
    c.Add(T::BGR, GL_BGR);
    c.Add(T::BGR_Int, GL_BGR_INTEGER);
    c.Add(T::RGBA, GL_RGBA);
    c.Add(T::RGBA_Int, GL_RGBA_INTEGER);
    c.Add(T::BGRA, GL_BGRA);
    c.Add(T::BGRA_Int, GL_BGRA_INTEGER);
    c.Add(T::DepthStencil, GL_DEPTH_STENCIL);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{
[[nodiscard]] inline constexpr auto ToGlValue(GlPixelBufferLayout v)
{
    return detail::kGlPixelBufferLayoutToGlValue.to_gl_value.Get(v);
}
}  // namespace klgl
