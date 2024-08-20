#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureFilterToGlValue = []
{
    using T = GlTextureFilter;
    ass::EnumMap<T, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 6);

    add(T::Nearest, GL_NEAREST);
    add(T::Linear, GL_LINE);
    add(T::NearestMipmapNearest, GL_NEAREST_MIPMAP_NEAREST);
    add(T::LinearMipmapNearest, GL_LINEAR_MIPMAP_NEAREST);
    add(T::NearestMipmapLinear, GL_NEAREST_MIPMAP_LINEAR);
    add(T::LinearMipmapLinear, GL_LINEAR_MIPMAP_LINEAR);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLint ToGlValue(GlTextureFilter filter) noexcept
{
    return detail::kGlTextureFilterToGlValue.Get(filter);
}

}  // namespace klgl
