#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTargetTextureType = []
{
    using T = GlTargetTextureType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 10);
    c.Add(T::Texture1d, GL_TEXTURE_1D);
    c.Add(T::Texture1dArray, GL_TEXTURE_1D_ARRAY);
    c.Add(T::Texture2d, GL_TEXTURE_2D);
    c.Add(T::Texture2dArray, GL_TEXTURE_2D_ARRAY);
    c.Add(T::Texture2dMultisample, GL_TEXTURE_2D_MULTISAMPLE);
    c.Add(T::Texture2dMultisampleArray, GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
    c.Add(T::Texture3d, GL_TEXTURE_3D);
    c.Add(T::TextureCubeMap, GL_TEXTURE_CUBE_MAP);
    c.Add(T::TextureCubeMapArray, GL_TEXTURE_CUBE_MAP_ARRAY);
    c.Add(T::TextureRectangle, GL_TEXTURE_RECTANGLE);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr auto ToGlValue(GlTargetTextureType buffer_type) noexcept
{
    return detail::kGlTargetTextureType.to_gl_value.Get(buffer_type);
}

}  // namespace klgl
