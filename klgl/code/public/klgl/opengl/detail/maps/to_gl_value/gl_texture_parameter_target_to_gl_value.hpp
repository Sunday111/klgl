#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureParameterTargetToGlValue = []
{
    using T = GlTextureParameterTarget;
    ass::EnumMap<T, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 10);

    add(T::Texture1d, GL_TEXTURE_1D);
    add(T::Texture1dArray, GL_TEXTURE_1D_ARRAY);
    add(T::Texture2d, GL_TEXTURE_2D);
    add(T::Texture2dArray, GL_TEXTURE_2D_ARRAY);
    add(T::Texture2dMultisample, GL_TEXTURE_2D_MULTISAMPLE);
    add(T::Texture2dMultisampleArray, GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
    add(T::Texture3d, GL_TEXTURE_3D);
    add(T::TextureCubeMap, GL_TEXTURE_CUBE_MAP);
    add(T::TextureCubeMapArray, GL_TEXTURE_CUBE_MAP_ARRAY);
    add(T::TextureRectangle, GL_TEXTURE_RECTANGLE);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlTextureParameterTarget buffer_type) noexcept
{
    return detail::kGlTextureParameterTargetToGlValue.Get(buffer_type);
}

}  // namespace klgl
