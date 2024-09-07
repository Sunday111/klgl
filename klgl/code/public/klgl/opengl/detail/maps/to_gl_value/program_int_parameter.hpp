#pragma once

#include <ass/enum_map.hpp>

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlProgramIntParameterToGlValue = []
{
    using T = GlProgramIntParameter;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 20);

    c.Add(T::DeleteStatus, GL_DELETE_STATUS);
    c.Add(T::LinkStatus, GL_LINK_STATUS);
    c.Add(T::ValidateStatus, GL_VALIDATE_STATUS);
    c.Add(T::InfoLogLength, GL_INFO_LOG_LENGTH);
    c.Add(T::AttachedShaders, GL_ATTACHED_SHADERS);
    c.Add(T::ActiveAtomicCounterBuffers, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS);
    c.Add(T::ActiveAttributes, GL_ACTIVE_ATTRIBUTES);
    c.Add(T::ActiveAttributeMaxLength, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
    c.Add(T::ActiveUniforms, GL_ACTIVE_UNIFORMS);
    c.Add(T::ActiveUniformBlocks, GL_ACTIVE_UNIFORM_BLOCKS);
    c.Add(T::ActiveUniformBlockMaxNameLength, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH);
    c.Add(T::ActiveUniformMaxLength, GL_ACTIVE_UNIFORM_MAX_LENGTH);
    c.Add(T::ComputeWorkGroupSize, GL_COMPUTE_WORK_GROUP_SIZE);
    c.Add(T::ProgramBinaryLength, GL_PROGRAM_BINARY_LENGTH);
    c.Add(T::TransformFeedbackBufferMode, GL_TRANSFORM_FEEDBACK_BUFFER_MODE);
    c.Add(T::TransformFeedbackVaryings, GL_TRANSFORM_FEEDBACK_VARYINGS);
    c.Add(T::TransformFeedbackVaryingMaxLength, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH);
    c.Add(T::GeometryVerticesOut, GL_GEOMETRY_VERTICES_OUT);
    c.Add(T::GeometryInputType, GL_GEOMETRY_INPUT_TYPE);
    c.Add(T::GeometryOutputType, GL_GEOMETRY_OUTPUT_TYPE);

    return c;
}();
}  // namespace klgl::detail

namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlProgramIntParameter parameter) noexcept
{
    return detail::kGlProgramIntParameterToGlValue.to_gl_value.Get(parameter);
}

}  // namespace klgl
