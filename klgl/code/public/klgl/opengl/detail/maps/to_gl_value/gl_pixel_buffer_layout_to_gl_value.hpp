#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{

inline constexpr auto kGlPixelBufferChannelTypeToGlValue = []
{
    ass::EnumMap<GlPixelBufferChannelType, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlPixelBufferChannelType, 2);
    add(GlPixelBufferChannelType::UByte, GL_UNSIGNED_BYTE);
    add(GlPixelBufferChannelType::Float, GL_FLOAT);

    return map;
}();
}  // namespace klgl::detail

namespace klgl
{
[[nodiscard]] inline constexpr auto ToGlValue(GlPixelBufferChannelType v)
{
    return detail::kGlPixelBufferChannelTypeToGlValue.Get(v);
}
}  // namespace klgl
