#pragma once

#include "klgl/opengl/enums.hpp"
#include "maps/polygon_mode_to_gl_value.hpp"
#include "maps/polygon_mode_to_string.hpp"

namespace klgl
{

constexpr GLenum ToGlValue(GlPolygonMode mode) noexcept
{
    return detail::kGlPolygonModelToGlValue.Get(mode);
}

constexpr std::string_view ToString(GlPolygonMode v)
{
    return detail::kGlPolygonModeToString.Get(v);
}

constexpr GLenum ToGlValue(GlTextureWrap wrap) noexcept
{
    static_assert(static_cast<std::underlying_type_t<GlTextureWrap>>(GlTextureWrap::kMax) == 3);
    switch (wrap)
    {
    case GlTextureWrap::S:
        return GL_TEXTURE_WRAP_S;
    case GlTextureWrap::T:
        return GL_TEXTURE_WRAP_T;
    default:
        return GL_TEXTURE_WRAP_T;
    }
}

constexpr GLint ToGlValue(GlTextureWrapMode mode) noexcept
{
    static_assert(static_cast<std::underlying_type_t<GlTextureWrapMode>>(GlTextureWrapMode::kMax) == 5);
    switch (mode)
    {
    case GlTextureWrapMode::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
        break;
    case GlTextureWrapMode::ClampToBorder:
        return GL_CLAMP_TO_BORDER;
        break;
    case GlTextureWrapMode::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
        break;
    case GlTextureWrapMode::Repeat:
        return GL_REPEAT;
        break;
    default:
        return GL_MIRROR_CLAMP_TO_EDGE;
        break;
    }
}

constexpr GLint ToGlValue(GlTextureFilter mode) noexcept
{
    static_assert(static_cast<std::underlying_type_t<GlTextureFilter>>(GlTextureFilter::kMax) == 6);
    switch (mode)
    {
    case GlTextureFilter::Nearest:
        return GL_NEAREST;
        break;
    case GlTextureFilter::Linear:
        return GL_LINEAR;
        break;
    case GlTextureFilter::NearestMipmapNearest:
        return GL_NEAREST_MIPMAP_NEAREST;
        break;
    case GlTextureFilter::LinearMipmapNearest:
        return GL_LINEAR_MIPMAP_NEAREST;
        break;
    case GlTextureFilter::NearestMipmapLinear:
        return GL_NEAREST_MIPMAP_LINEAR;
        break;
    default:
        return GL_LINEAR_MIPMAP_LINEAR;
        break;
    }
}
}  // namespace klgl
