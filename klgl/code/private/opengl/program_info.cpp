#include "klgl/opengl/program_info.hpp"

#include <algorithm>
#include <ranges>

#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"

namespace klgl
{
void GlProgramInfo::FetchVertexAttributes(GlProgramId program)
{
    const size_t num_attributes = OpenGl::GetProgramActiveAttributesCount(program);

    std::string attribute_name_buffer;
    attribute_name_buffer.resize(OpenGl::GetProgramActiveAttributeMaxNameLength(program));

    vertex_attributes.resize(num_attributes);

    for (const size_t attribute_index : std::views::iota(size_t{0}, num_attributes))
    {
        auto& attribute = vertex_attributes[attribute_index];

        size_t attribute_name_length = 0;
        OpenGl::GetActiveAttribute(
            program,
            attribute_index,
            attribute_name_buffer.size(),
            attribute_name_length,
            attribute.size,
            attribute.type,
            attribute_name_buffer.data());

        attribute.index = attribute_index;
        attribute.name = std::string_view{attribute_name_buffer}.substr(0, attribute_name_length);
        attribute.location = OpenGl::GetAttributeLocation(program, attribute.name);
    }
}

void GlProgramInfo::FetchUniforms(GlProgramId program)
{
    const size_t num_uniforms = OpenGl::GetProgramActiveUniformsCount(program);
    uniforms.resize(num_uniforms);

    std::string name_buffer;
    name_buffer.resize(OpenGl::GetProgramActiveUniformMaxNameLength(program));

    for (const size_t index : std::views::iota(size_t{0}, num_uniforms))
    {
        auto& uniform = uniforms[index];

        size_t name_length = 0;
        OpenGl::GetActiveUniform(
            program,
            index,
            name_buffer.size(),
            name_length,
            uniform.size,
            uniform.type,
            name_buffer.data());

        uniform.index = index;
        uniform.name = std::string_view{name_buffer}.substr(0, name_length);
        int32_t location = OpenGl::GetUniformLocation(program, uniform.name.data());
        klgl::ErrorHandling::Ensure(location >= 0, "Unexpected negative location");
        uniform.location = static_cast<size_t>(location);
    }
}

size_t GlProgramInfo::VerifyAndGetVertexAttributeLocation(std::string_view name, GlVertexAttributeType type) const
{
    auto it = std::ranges::find(vertex_attributes, name, &GlVertexAttributeInfo::name);
    klgl::ErrorHandling::Ensure(it != vertex_attributes.end(), "Could not find vertex attribute {}", name);
    klgl::ErrorHandling::Ensure(
        it->type == type,
        "Vertex attribute \"{}\" has type {} but you expect it to be {}",
        name,
        it->type,
        type);
    return it->location;
}

}  // namespace klgl
