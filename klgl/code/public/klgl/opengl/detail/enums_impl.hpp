#pragma once

#include <string_view>

#include "CppReflection/GetStaticTypeInfo.hpp"
#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/enums.hpp"
#include "maps/polygon_mode_to_gl_value.hpp"

namespace klgl
{

constexpr GLenum ToGlValue(GlPolygonMode mode) noexcept
{
    return detail::kGlPolygonModelToGlValue.Get(mode);
}

constexpr GLenum ToGlValue(GlTextureWrapAxis wrap) noexcept
{
    KLGL_ENSURE_ENUM_SIZE(GlTextureWrapAxis, 3);
    switch (wrap)
    {
    case GlTextureWrapAxis::S:
        return GL_TEXTURE_WRAP_S;
    case GlTextureWrapAxis::T:
        return GL_TEXTURE_WRAP_T;
    default:
        return GL_TEXTURE_WRAP_T;
    }
}

constexpr GLint ToGlValue(GlTextureWrapMode mode) noexcept
{
    KLGL_ENSURE_ENUM_SIZE(GlTextureWrapMode, 5);
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
    KLGL_ENSURE_ENUM_SIZE(GlTextureFilter, 6);
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

namespace cppreflection
{
template <>
struct TypeReflectionProvider<::klgl::GlPolygonMode>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlPolygonMode>(
                   "GlPolygonMode",
                   edt::GUID::Create("B6808C23-A1BA-42A5-AA31-F08ED15D3AC9"))
            .Value(::klgl::GlPolygonMode::Point, "Point")
            .Value(::klgl::GlPolygonMode::Line, "Line")
            .Value(::klgl::GlPolygonMode::Fill, "Fill");
    }
};

template <>
struct TypeReflectionProvider<::klgl::GlTextureWrapAxis>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlTextureWrapAxis>(
                   "GlTextureWrap",
                   edt::GUID::Create("8D676F4B-F1B5-4F80-8772-125376832D8E"))
            .Value(::klgl::GlTextureWrapAxis::S, "S")
            .Value(::klgl::GlTextureWrapAxis::T, "T")
            .Value(::klgl::GlTextureWrapAxis::R, "R");
    }
};

template <>
struct TypeReflectionProvider<::klgl::GlTextureWrapMode>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlTextureWrapMode>(
                   "GlTextureWrapMode",
                   edt::GUID::Create("668C28DB-01FD-47DE-A3B9-7081A1B68CC4"))
            .Value(::klgl::GlTextureWrapMode::ClampToEdge, "ClampToEdge")
            .Value(::klgl::GlTextureWrapMode::ClampToBorder, "ClampToBorder")
            .Value(::klgl::GlTextureWrapMode::MirroredRepeat, "MirroredRepeat")
            .Value(::klgl::GlTextureWrapMode::Repeat, "Repeat")
            .Value(::klgl::GlTextureWrapMode::MirrorClampToEdge, "MirrorClampToEdge");
    }
};

template <>
struct TypeReflectionProvider<::klgl::GlTextureFilter>
{
    [[nodiscard]] inline constexpr static auto ReflectType()
    {
        return cppreflection::StaticEnumTypeInfo<::klgl::GlTextureFilter>(
                   "GlTextureFilter",
                   edt::GUID::Create("73D29DE8-C8B0-4C97-897B-1154C0D0ABBB"))
            .Value(::klgl::GlTextureFilter::Nearest, "Nearest")
            .Value(::klgl::GlTextureFilter::Linear, "Linear")
            .Value(::klgl::GlTextureFilter::NearestMipmapNearest, "NearestMipmapNearest")
            .Value(::klgl::GlTextureFilter::LinearMipmapNearest, "LinearMipmapNearest")
            .Value(::klgl::GlTextureFilter::NearestMipmapLinear, "NearestMipmapLinear")
            .Value(::klgl::GlTextureFilter::LinearMipmapLinear, "LinearMipmapLinear");
    }
};
}  // namespace cppreflection
