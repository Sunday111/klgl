#pragma once

#include "ass/enum_map.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlTextureWrapModeToGlValue = []
{
    using T = GlTextureWrapMode;
    ass::EnumMap<T, GLint> map;

    auto add = [&](auto key, GLint value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(T, 5);

    add(T::ClampToEdge, GL_CLAMP_TO_EDGE);
    add(T::ClampToBorder, GL_CLAMP_TO_BORDER);
    add(T::MirroredRepeat, GL_MIRRORED_REPEAT);
    add(T::Repeat, GL_REPEAT);
    add(T::MirrorClampToEdge, GL_MIRROR_CLAMP_TO_EDGE);

    return map;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLint ToGlValue(GlTextureWrapMode wrap_mode) noexcept
{
    return detail::kGlTextureWrapModeToGlValue.Get(wrap_mode);
}

}  // namespace klgl
