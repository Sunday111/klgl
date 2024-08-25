#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTexParamType = []
{
    using T = GlTextureParameterType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 19);
    c.Add(T::DepthStencilMode, GL_DEPTH_STENCIL_TEXTURE_MODE);
    c.Add(T::BaseLevel, GL_TEXTURE_BASE_LEVEL);
    c.Add(T::ComapreFunction, GL_TEXTURE_COMPARE_FUNC);
    c.Add(T::CompareMode, GL_TEXTURE_COMPARE_MODE);
    c.Add(T::LoadBias, GL_TEXTURE_LOD_BIAS);
    c.Add(T::MinificationFilter, GL_TEXTURE_MIN_FILTER);
    c.Add(T::MagnificationFilter, GL_TEXTURE_MAG_FILTER);
    c.Add(T::MinLOD, GL_TEXTURE_MIN_LOD);
    c.Add(T::MaxLOD, GL_TEXTURE_MAX_LOD);
    c.Add(T::MaxLevel, GL_TEXTURE_MAX_LEVEL);
    c.Add(T::SwizzleR, GL_TEXTURE_SWIZZLE_R);
    c.Add(T::SwizzleG, GL_TEXTURE_SWIZZLE_G);
    c.Add(T::SwizzleB, GL_TEXTURE_SWIZZLE_B);
    c.Add(T::SwizzleA, GL_TEXTURE_SWIZZLE_A);
    c.Add(T::WrapS, GL_TEXTURE_WRAP_S);
    c.Add(T::WrapT, GL_TEXTURE_WRAP_T);
    c.Add(T::WrapR, GL_TEXTURE_WRAP_R);
    c.Add(T::BorderColor, GL_TEXTURE_BORDER_COLOR);
    c.Add(T::SwizzleRGBA, GL_TEXTURE_SWIZZLE_RGBA);
    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlTextureParameterType buffer_type) noexcept
{
    return detail::kGlTexParamType.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
