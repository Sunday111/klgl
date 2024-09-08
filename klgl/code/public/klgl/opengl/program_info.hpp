#pragma once

#include <string>

#include "klgl/opengl/detail/maps/type_to_vertex_attribute_type.hpp"
#include "klgl/opengl/enums.hpp"
#include "klgl/opengl/identifiers.hpp"

namespace klgl
{
struct GlVertexAttributeInfo
{
    std::string name;
    size_t index{};
    size_t location{};
    size_t size{};
    GlVertexAttributeType type{};
};

struct GlUniformInfo
{
    std::string name;
    size_t index{};
    size_t size{};
    GlUniformType type{};
};

struct GlProgramInfo
{
    explicit GlProgramInfo(GlProgramId program_id) : program(program_id) {}

    void FetchVertexAttributes();
    void FetchUniforms();

    [[nodiscard]] size_t VerifyAndGetVertexAttributeLocation(std::string_view name, GlVertexAttributeType type) const;

    template <typename T>
    [[nodiscard]] size_t VerifyAndGetVertexAttributeLocation(std::string_view name) const
    {
        return VerifyAndGetVertexAttributeLocation(name, detail::TypeVertAttribTypeEnum<T>::value);
    }

    GlProgramId program;
    std::vector<GlVertexAttributeInfo> vertex_attributes;
    std::vector<GlUniformInfo> uniforms;
};
}  // namespace klgl
