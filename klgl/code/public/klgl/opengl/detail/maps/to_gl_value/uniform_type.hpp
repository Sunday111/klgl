#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlUniformTypeToGlValue = []
{
    using T = GlUniformType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 105);
    c.Add(T::Float, GL_FLOAT);
    c.Add(T::FloatVec2, GL_FLOAT_VEC2);
    c.Add(T::FloatVec3, GL_FLOAT_VEC3);
    c.Add(T::FloatVec4, GL_FLOAT_VEC4);
    c.Add(T::Double, GL_DOUBLE);
    c.Add(T::DoubleVec2, GL_DOUBLE_VEC2);
    c.Add(T::DoubleVec3, GL_DOUBLE_VEC3);
    c.Add(T::DoubleVec4, GL_DOUBLE_VEC4);
    c.Add(T::Int, GL_INT);
    c.Add(T::IntVec2, GL_INT_VEC2);
    c.Add(T::IntVec3, GL_INT_VEC3);
    c.Add(T::IntVec4, GL_INT_VEC4);
    c.Add(T::UnsignedInt, GL_UNSIGNED_INT);
    c.Add(T::UnsignedIntVec2, GL_UNSIGNED_INT_VEC2);
    c.Add(T::UnsignedIntVec3, GL_UNSIGNED_INT_VEC3);
    c.Add(T::UnsignedIntVec4, GL_UNSIGNED_INT_VEC4);
    c.Add(T::Bool, GL_BOOL);
    c.Add(T::BoolVec2, GL_BOOL_VEC2);
    c.Add(T::BoolVec3, GL_BOOL_VEC3);
    c.Add(T::BoolVec4, GL_BOOL_VEC4);
    c.Add(T::FloatMat2, GL_FLOAT_MAT2);
    c.Add(T::FloatMat3, GL_FLOAT_MAT3);
    c.Add(T::FloatMat4, GL_FLOAT_MAT4);
    c.Add(T::FloatMat2x3, GL_FLOAT_MAT2x3);
    c.Add(T::FloatMat2x4, GL_FLOAT_MAT2x4);
    c.Add(T::FloatMat3x2, GL_FLOAT_MAT3x2);
    c.Add(T::FloatMat3x4, GL_FLOAT_MAT3x4);
    c.Add(T::FloatMat4x2, GL_FLOAT_MAT4x2);
    c.Add(T::FloatMat4x3, GL_FLOAT_MAT4x3);
    c.Add(T::DoubleMat2, GL_DOUBLE_MAT2);
    c.Add(T::DoubleMat3, GL_DOUBLE_MAT3);
    c.Add(T::DoubleMat4, GL_DOUBLE_MAT4);
    c.Add(T::DoubleMat2x3, GL_DOUBLE_MAT2x3);
    c.Add(T::DoubleMat2x4, GL_DOUBLE_MAT2x4);
    c.Add(T::DoubleMat3x2, GL_DOUBLE_MAT3x2);
    c.Add(T::DoubleMat3x4, GL_DOUBLE_MAT3x4);
    c.Add(T::DoubleMat4x2, GL_DOUBLE_MAT4x2);
    c.Add(T::DoubleMat4x3, GL_DOUBLE_MAT4x3);
    c.Add(T::Sampler1d, GL_SAMPLER_1D);
    c.Add(T::Sampler2d, GL_SAMPLER_2D);
    c.Add(T::Sampler3d, GL_SAMPLER_3D);
    c.Add(T::Samplercube, GL_SAMPLER_CUBE);
    c.Add(T::Sampler1dShadow, GL_SAMPLER_1D_SHADOW);
    c.Add(T::Sampler2dShadow, GL_SAMPLER_2D_SHADOW);
    c.Add(T::Sampler1dArray, GL_SAMPLER_1D_ARRAY);
    c.Add(T::Sampler2dArray, GL_SAMPLER_2D_ARRAY);
    c.Add(T::Sampler1dArray_shadow, GL_SAMPLER_1D_ARRAY_SHADOW);
    c.Add(T::Sampler2dArray_shadow, GL_SAMPLER_2D_ARRAY_SHADOW);
    c.Add(T::Sampler2dMultisample, GL_SAMPLER_2D_MULTISAMPLE);
    c.Add(T::Sampler2dMultisample_array, GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
    c.Add(T::SamplerCubeShadow, GL_SAMPLER_CUBE_SHADOW);
    c.Add(T::SamplerBuffer, GL_SAMPLER_BUFFER);
    c.Add(T::Sampler2dRect, GL_SAMPLER_2D_RECT);
    c.Add(T::Sampler2dRectShadow, GL_SAMPLER_2D_RECT_SHADOW);
    c.Add(T::IntSampler1d, GL_INT_SAMPLER_1D);
    c.Add(T::IntSampler2d, GL_INT_SAMPLER_2D);
    c.Add(T::IntSampler3d, GL_INT_SAMPLER_3D);
    c.Add(T::IntSamplerCube, GL_INT_SAMPLER_CUBE);
    c.Add(T::IntSampler1dArray, GL_INT_SAMPLER_1D_ARRAY);
    c.Add(T::IntSampler2dArray, GL_INT_SAMPLER_2D_ARRAY);
    c.Add(T::IntSampler2dMultisample, GL_INT_SAMPLER_2D_MULTISAMPLE);
    c.Add(T::IntSampler2dMultisample_array, GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
    c.Add(T::IntSamplerBuffer, GL_INT_SAMPLER_BUFFER);
    c.Add(T::IntSampler2dRect, GL_INT_SAMPLER_2D_RECT);
    c.Add(T::UnsignedIntSampler1d, GL_UNSIGNED_INT_SAMPLER_1D);
    c.Add(T::UnsignedIntSampler2d, GL_UNSIGNED_INT_SAMPLER_2D);
    c.Add(T::UnsignedIntSampler3d, GL_UNSIGNED_INT_SAMPLER_3D);
    c.Add(T::UnsignedIntSamplerCube, GL_UNSIGNED_INT_SAMPLER_CUBE);
    c.Add(T::UnsignedIntSampler1dArray, GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
    c.Add(T::UnsignedIntSampler2dArray, GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
    c.Add(T::UnsignedIntSampler2dMultisample, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
    c.Add(T::UnsignedIntSampler2dMultisampleArray, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
    c.Add(T::UnsignedIntSamplerBuffer, GL_UNSIGNED_INT_SAMPLER_BUFFER);
    c.Add(T::UnsignedIntSampler2dRect, GL_UNSIGNED_INT_SAMPLER_2D_RECT);
    c.Add(T::Image1d, GL_IMAGE_1D);
    c.Add(T::Image2d, GL_IMAGE_2D);
    c.Add(T::Image3d, GL_IMAGE_3D);
    c.Add(T::Image2dRect, GL_IMAGE_2D_RECT);
    c.Add(T::ImageCube, GL_IMAGE_CUBE);
    c.Add(T::ImageBuffer, GL_IMAGE_BUFFER);
    c.Add(T::Image1dArray, GL_IMAGE_1D_ARRAY);
    c.Add(T::Image2dArray, GL_IMAGE_2D_ARRAY);
    c.Add(T::Image2dMultisample, GL_IMAGE_2D_MULTISAMPLE);
    c.Add(T::Image2dMultisample_array, GL_IMAGE_2D_MULTISAMPLE_ARRAY);
    c.Add(T::IntImage1d, GL_INT_IMAGE_1D);
    c.Add(T::IntImage2d, GL_INT_IMAGE_2D);
    c.Add(T::IntImage3d, GL_INT_IMAGE_3D);
    c.Add(T::IntImage2dRect, GL_INT_IMAGE_2D_RECT);
    c.Add(T::IntImageCube, GL_INT_IMAGE_CUBE);
    c.Add(T::IntImageBuffer, GL_INT_IMAGE_BUFFER);
    c.Add(T::IntImage1dArray, GL_INT_IMAGE_1D_ARRAY);
    c.Add(T::IntImage2dArray, GL_INT_IMAGE_2D_ARRAY);
    c.Add(T::IntImage2dMultisample, GL_INT_IMAGE_2D_MULTISAMPLE);
    c.Add(T::IntImage2dMultisampleArray, GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
    c.Add(T::UnsignedIntImage1d, GL_UNSIGNED_INT_IMAGE_1D);
    c.Add(T::UnsignedIntImage2d, GL_UNSIGNED_INT_IMAGE_2D);
    c.Add(T::UnsignedIntImage3d, GL_UNSIGNED_INT_IMAGE_3D);
    c.Add(T::UnsignedIntImage2dRect, GL_UNSIGNED_INT_IMAGE_2D_RECT);
    c.Add(T::UnsignedIntImageCube, GL_UNSIGNED_INT_IMAGE_CUBE);
    c.Add(T::UnsignedIntImageBuffer, GL_UNSIGNED_INT_IMAGE_BUFFER);
    c.Add(T::UnsignedIntImage1dArray, GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
    c.Add(T::UnsignedIntImage2dArray, GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
    c.Add(T::UnsignedIntImage2dMultisample, GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
    c.Add(T::UnsignedIntImage2dMultisampleArray, GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
    c.Add(T::UnsignedIntAtomicCounter, GL_UNSIGNED_INT_ATOMIC_COUNTER);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlUniformType uniform_type) noexcept
{
    return detail::kGlUniformTypeToGlValue.to_gl_value.Get(uniform_type);
}

}  // namespace klgl
