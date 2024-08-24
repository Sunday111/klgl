#pragma once

#include "klgl/macro/ensure_enum_size.hpp"
#include "klgl/opengl/detail/maps/to_gl_value/opengl_value_converter.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl::detail
{
inline constexpr auto kGlShaderTypeToGlValue = []
{
    using T = GlShaderType;
    OpenGlValueConverter<T, GLenum> c;

    KLGL_ENSURE_ENUM_SIZE(T, 6);
    c.Add(T::Compute, GL_COMPUTE_SHADER);
    c.Add(T::Vertex, GL_VERTEX_SHADER);
    c.Add(T::TesselationControl, GL_TESS_CONTROL_SHADER);
    c.Add(T::TesselationEvaluation, GL_TESS_EVALUATION_SHADER);
    c.Add(T::Geometry, GL_GEOMETRY_SHADER);
    c.Add(T::Fragment, GL_FRAGMENT_SHADER);

    return c;
}();
}  // namespace klgl::detail
namespace klgl
{

[[nodiscard]] constexpr GLenum ToGlValue(GlShaderType shader_type) noexcept
{
    return detail::kGlShaderTypeToGlValue.to_gl_value.Get(shader_type);
}

}  // namespace klgl
