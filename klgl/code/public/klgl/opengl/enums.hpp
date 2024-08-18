#pragma once

#include <string_view>

#include "glad/glad.h"
#include "klgl/formatting/simple_format.hpp"
#include "klgl/macro/enum_as_index.hpp"

namespace klgl
{

enum class GlPixelBufferLayout
{
    R,
    R_Int,
    Depth,
    RG,
    RG_Int,
    RGB,
    RGB_Int,
    BGR,
    BGR_Int,
    RGBA,
    RGBA_Int,
    BGRA,
    BGRA_Int,
    DepthStencil,

    kMax
};

enum class GlPixelBufferChannelType
{
    UByte,
    Float,

    kMax
};

enum class GlPolygonMode : uint8_t
{
    Point,
    Line,
    Fill,

    kMax
};

enum class GlTextureWrap : uint8_t
{
    S,
    T,
    R,
    kMax
};

enum class GlTextureWrapMode : uint8_t
{
    ClampToEdge,
    ClampToBorder,
    MirroredRepeat,
    Repeat,
    MirrorClampToEdge,
    kMax
};

enum class GlTextureFilter : uint8_t
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
    kMax
};

enum class GlTextureInternalFormat
{
    DEPTH_COMPONENT,
    DEPTH_STENCIL,
    RED,
    RG,
    RGB,
    RGBA,

    R8,
    R8_SNORM,
    R16,
    R16_SNORM,
    RG8,
    RG8_SNORM,
    RG16,
    RG16_SNORM,
    R3_G3_B2,
    RGB4,
    RGB5,
    RGB8,
    RGB8_SNORM,
    RGB10,
    RGB12,
    RGB16_SNORM,
    RGBA2,
    RGBA4,
    RGB5_A1,
    RGBA8,
    RGBA8_SNORM,
    RGB10_A2,
    RGB10_A2UI,
    RGBA12,
    RGBA16,
    SRGB8,
    SRGB8_ALPHA8,
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
    R11F_G11F_B10F,
    RGB9_E5,
    R8I,
    R8UI,
    R16I,
    R16UI,
    R32I,
    R32UI,
    RG8I,
    RG8UI,
    RG16I,
    RG16UI,
    RG32I,
    RG32UI,
    RGB8I,
    RGB8UI,
    RGB16I,
    RGB16UI,
    RGB32I,
    RGB32UI,
    RGBA8I,
    RGBA8UI,
    RGBA16I,
    RGBA16UI,
    RGBA32I,
    RGBA32UI,

    kMax
};

std::string_view ToString(GlPixelBufferLayout);
std::string_view ToString(GlPixelBufferChannelType);
std::string_view ToString(GlTextureInternalFormat);
constexpr std::string_view ToString(GlPolygonMode);

GLint ToGlValue(GlPixelBufferLayout);
GLenum ToGlValue(GlPixelBufferChannelType);
GLint ToGlValue(GlTextureInternalFormat);
constexpr GLenum ToGlValue(GlPolygonMode mode) noexcept;

constexpr GLenum ToGlValue(GlTextureWrap wrap) noexcept;
constexpr GLint ToGlValue(GlTextureWrapMode mode) noexcept;
constexpr GLint ToGlValue(GlTextureFilter mode) noexcept;

}  // namespace klgl

KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX(GlPixelBufferLayout);
KLGL_MAKE_ENUM_FORMATTER(GlPixelBufferLayout, klgl::ToString);

KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX(GlPixelBufferChannelType);
KLGL_MAKE_ENUM_FORMATTER(GlPixelBufferChannelType, klgl::ToString);

KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX(GlTextureInternalFormat);
KLGL_MAKE_ENUM_FORMATTER(GlTextureInternalFormat, klgl::ToString);

KLGL_ENUM_AS_INDEX_ZERO_TO_KMAX(GlPolygonMode);
KLGL_MAKE_ENUM_FORMATTER(GlPolygonMode, klgl::ToString);

#include "detail/enums_impl.hpp"  // IWYU pragma: keep
