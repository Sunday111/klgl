#pragma once

#include "glad/glad.h"
#include "klgl/formatting/simple_format.hpp"
#include "klgl/macro/enum_as_index.hpp"

namespace klgl
{

enum class GlError
{
    NoError,
    InvalidEnum,
    InvalidValue,
    InvalidOperation,
    StackOverflow,
    StackUnderflow,
    OutOfMemory,
    Unknown
};

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
};

enum class GlPixelBufferChannelType
{
    UByte,
    Float,
};

enum class GlPolygonMode : uint8_t
{
    Point,
    Line,
    Fill,

};

enum class GlTextureWrapAxis : uint8_t
{
    S,
    T,
    R,
};

enum class GlTextureWrapMode : uint8_t
{
    ClampToEdge,
    ClampToBorder,
    MirroredRepeat,
    Repeat,
    MirrorClampToEdge,
};

enum class GlTextureFilter : uint8_t
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
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
};

enum class GlBufferType
{
    Array,
    AtomicCounter,
    CopyRead,
    CopyWrite,
    DispatchIndirect,
    DrawIndirect,
    ElementArray,
    PixelPack,
    PixelUnpack,
    Query,
    ShaderStorage,
    Texture,
    TransformFeedback,
    Uniform
};

enum class GlUsage
{
    StreamDraw,
    StreamRead,
    StreamCopy,
    StaticDraw,
    StaticRead,
    StaticCopy,
    DynamicDraw,
    DynamicRead,
    DynamicCopy
};

enum class GlPrimitiveType
{
    Points,
    LineStrip,
    LineLoop,
    Lines,
    LineStripAdjacency,
    LinesAdjacency,
    TriangleStrip,
    TriangleFan,
    Triangles,
    TriangleStripAdjacency,
    TrianglesAdjacency,
    Patches
};

enum class GlIndexBufferElementType
{
    UnsignedByte,
    UnsignedShort,
    UnsignedInt
};

enum class GlVertexAttribComponentType
{
    // glVertexAttribPointer and glVertexAttribIPointer
    Byte,
    UnsignedByte,
    Short,
    UnsignedShort,
    Int,
    UnsignedInt,

    // glVertexAttribPointer only
    HalfFloat,
    Float,
    Double,  // the only param for glVertexAttribLPointer
    Fixed,
    Int_2_10_10_10_Rev,
    UnsignedInt_2_10_10_10_Rev,
    UnsignedInt_10F_11F_11F_Rev,
};

enum class GlTargetTextureType
{
    Texture1d,
    Texture1dArray,
    Texture2d,
    Texture2dArray,
    Texture2dMultisample,
    Texture2dMultisampleArray,
    Texture3d,
    TextureCubeMap,
    TextureCubeMapArray,
    TextureRectangle
};

// Specifies the comparison operator used when GL_TEXTURE_COMPARE_MODE is set to GL_COMPARE_REF_TO_TEXTURE.
enum class GlDepthTextureCompareFunction
{
    LessOrEqual,
    GreaterOrEqual,
    Less,
    Greater,
    Equal,
    NotEqual,
    Always,
    Never,
};

enum class GlDepthTextureCompareMode
{
    // Specifies that the interpolated and clamped r texture coordinate should be compared to the value in the currently
    // bound depth texture. Compare function determines how the comparison is evaluated
    RefToTexture,

    // Specifies that the red channel should be assigned the appropriate value from the currently bound depth texture.
    None
};

enum class GlShaderType
{
    Compute,
    Vertex,
    TesselationControl,
    TesselationEvaluation,
    Geometry,
    Fragment,
};

enum class GlTextureParameterType
{
    DepthStencilMode,
    BaseLevel,
    ComapreFunction,
    CompareMode,
    LoadBias,
    MinificationFilter,
    MagnificationFilter,
    MinLOD,
    MaxLOD,
    MaxLevel,
    SwizzleR,
    SwizzleG,
    SwizzleB,
    SwizzleA,
    WrapS,
    WrapT,
    WrapR,
    BorderColor,
    SwizzleRGBA,
};

enum class GlCullFaceMode
{
    Front,
    Back,
    Both
};

enum class GlProgramIntParameter
{
    DeleteStatus,
    LinkStatus,
    ValidateStatus,
    InfoLogLength,
    AttachedShaders,
    ActiveAtomicCounterBuffers,
    ActiveAttributes,
    ActiveAttributeMaxLength,
    ActiveUniforms,
    ActiveUniformBlocks,
    ActiveUniformBlockMaxNameLength,
    ActiveUniformMaxLength,
    ComputeWorkGroupSize,
    ProgramBinaryLength,
    TransformFeedbackBufferMode,
    TransformFeedbackVaryings,
    TransformFeedbackVaryingMaxLength,
    GeometryVerticesOut,
    GeometryInputType,
    GeometryOutputType
};

enum class GlVertexAttributeType
{
    Float,
    FloatVec2,
    FloatVec3,
    FloatVec4,
    FloatMat2,
    FloatMat3,
    FloatMat4,
    FloatMat2x3,
    FloatMat2x4,
    FloatMat3x2,
    FloatMat3x4,
    FloatMat4x2,
    FloatMat4x3,
    Int,
    IntVec2,
    IntVec3,
    IntVec4,
    UnsignedInt,
    UnsignedIntVec2,
    UnsignedIntVec3,
    UnsignedIntVec4,
    Double,
    DoubleVec2,
    DoubleVec3,
    DoubleVec4,
    DoubleMat2,
    DoubleMat3,
    DoubleMat4,
    DoubleMat2x3,
    DoubleMat2x4,
    DoubleMat3x2,
    DoubleMat3x4,
    DoubleMat4x2,
    DoubleMat4x3
};

enum class GlUniformType
{
    Float,
    FloatVec2,
    FloatVec3,
    FloatVec4,
    Double,
    DoubleVec2,
    DoubleVec3,
    DoubleVec4,
    Int,
    IntVec2,
    IntVec3,
    IntVec4,
    UnsignedInt,
    UnsignedIntVec2,
    UnsignedIntVec3,
    UnsignedIntVec4,
    Bool,
    BoolVec2,
    BoolVec3,
    BoolVec4,
    FloatMat2,
    FloatMat3,
    FloatMat4,
    FloatMat2x3,
    FloatMat2x4,
    FloatMat3x2,
    FloatMat3x4,
    FloatMat4x2,
    FloatMat4x3,
    DoubleMat2,
    DoubleMat3,
    DoubleMat4,
    DoubleMat2x3,
    DoubleMat2x4,
    DoubleMat3x2,
    DoubleMat3x4,
    DoubleMat4x2,
    DoubleMat4x3,
    Sampler1d,
    Sampler2d,
    Sampler3d,
    Samplercube,
    Sampler1dShadow,
    Sampler2dShadow,
    Sampler1dArray,
    Sampler2dArray,
    Sampler1dArray_shadow,
    Sampler2dArray_shadow,
    Sampler2dMultisample,
    Sampler2dMultisample_array,
    SamplerCubeShadow,
    SamplerBuffer,
    Sampler2dRect,
    Sampler2dRectShadow,
    IntSampler1d,
    IntSampler2d,
    IntSampler3d,
    IntSamplerCube,
    IntSampler1dArray,
    IntSampler2dArray,
    IntSampler2dMultisample,
    IntSampler2dMultisample_array,
    IntSamplerBuffer,
    IntSampler2dRect,
    UnsignedIntSampler1d,
    UnsignedIntSampler2d,
    UnsignedIntSampler3d,
    UnsignedIntSamplerCube,
    UnsignedIntSampler1dArray,
    UnsignedIntSampler2dArray,
    UnsignedIntSampler2dMultisample,
    UnsignedIntSampler2dMultisampleArray,
    UnsignedIntSamplerBuffer,
    UnsignedIntSampler2dRect,
    Image1d,
    Image2d,
    Image3d,
    Image2dRect,
    ImageCube,
    ImageBuffer,
    Image1dArray,
    Image2dArray,
    Image2dMultisample,
    Image2dMultisample_array,
    IntImage1d,
    IntImage2d,
    IntImage3d,
    IntImage2dRect,
    IntImageCube,
    IntImageBuffer,
    IntImage1dArray,
    IntImage2dArray,
    IntImage2dMultisample,
    IntImage2dMultisampleArray,
    UnsignedIntImage1d,
    UnsignedIntImage2d,
    UnsignedIntImage3d,
    UnsignedIntImage2dRect,
    UnsignedIntImageCube,
    UnsignedIntImageBuffer,
    UnsignedIntImage1dArray,
    UnsignedIntImage2dArray,
    UnsignedIntImage2dMultisample,
    UnsignedIntImage2dMultisampleArray,
    UnsignedIntAtomicCounter,
};

}  // namespace klgl

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlError);
KLGL_MAKE_ENUM_FORMATTER(GlError);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlPixelBufferLayout);
KLGL_MAKE_ENUM_FORMATTER(GlPixelBufferLayout);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlPixelBufferChannelType);
KLGL_MAKE_ENUM_FORMATTER(GlPixelBufferChannelType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlTextureInternalFormat);
KLGL_MAKE_ENUM_FORMATTER(GlTextureInternalFormat);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlPolygonMode);
KLGL_MAKE_ENUM_FORMATTER(GlPolygonMode);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlTextureWrapAxis);
KLGL_MAKE_ENUM_FORMATTER(GlTextureWrapAxis);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlTextureWrapMode);
KLGL_MAKE_ENUM_FORMATTER(GlTextureWrapMode);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlTextureFilter);
KLGL_MAKE_ENUM_FORMATTER(GlTextureFilter);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlBufferType);
KLGL_MAKE_ENUM_FORMATTER(GlBufferType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlUsage);
KLGL_MAKE_ENUM_FORMATTER(GlUsage);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlPrimitiveType);
KLGL_MAKE_ENUM_FORMATTER(GlPrimitiveType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlIndexBufferElementType);
KLGL_MAKE_ENUM_FORMATTER(GlIndexBufferElementType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlVertexAttribComponentType);
KLGL_MAKE_ENUM_FORMATTER(GlVertexAttribComponentType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlTargetTextureType);
KLGL_MAKE_ENUM_FORMATTER(GlTargetTextureType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlDepthTextureCompareMode);
KLGL_MAKE_ENUM_FORMATTER(GlDepthTextureCompareMode);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlDepthTextureCompareFunction);
KLGL_MAKE_ENUM_FORMATTER(GlDepthTextureCompareFunction);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlShaderType);
KLGL_MAKE_ENUM_FORMATTER(GlShaderType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlTextureParameterType);
KLGL_MAKE_ENUM_FORMATTER(GlTextureParameterType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlCullFaceMode);
KLGL_MAKE_ENUM_FORMATTER(GlCullFaceMode);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlProgramIntParameter);
KLGL_MAKE_ENUM_FORMATTER(GlProgramIntParameter);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlVertexAttributeType);
KLGL_MAKE_ENUM_FORMATTER(GlVertexAttributeType);

KLGL_ENUM_AS_INDEX_MAGIC_ENUM(GlUniformType);
KLGL_MAKE_ENUM_FORMATTER(GlUniformType);
