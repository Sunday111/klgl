#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlUsageToGlValue = []
{
    ass::EnumMap<GlUsage, GLenum> map;

    auto add = [&](auto key, GLenum value)
    {
        assert(!map.Contains(key));
        map.GetOrAdd(key) = value;
    };

    KLGL_ENSURE_ENUM_SIZE(GlUsage, 9);
    add(GlUsage::StreamDraw, GL_STREAM_DRAW);
    add(GlUsage::StreamRead, GL_STREAM_READ);
    add(GlUsage::StreamCopy, GL_STREAM_COPY);
    add(GlUsage::StaticDraw, GL_STATIC_DRAW);
    add(GlUsage::StaticRead, GL_STATIC_READ);
    add(GlUsage::StaticCopy, GL_STATIC_COPY);
    add(GlUsage::DynamicDraw, GL_DYNAMIC_DRAW);
    add(GlUsage::DynamicRead, GL_DYNAMIC_READ);
    add(GlUsage::DynamicCopy, GL_DYNAMIC_COPY);

    return map;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlUsage usage) noexcept
{
    return detail::kGlUsageToGlValue.Get(usage);
}

}  // namespace klgl
