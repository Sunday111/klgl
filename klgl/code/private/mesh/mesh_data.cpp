#include "klgl/mesh/mesh_data.hpp"

#include <ass/enum_set.hpp>

#include "klgl/error_handling.hpp"

namespace klgl
{

void MeshOpenGL::ValidateIndicesCountForTopology(const GlPrimitiveType topology, const size_t num_indices)
{
    switch (topology)
    {
    case GlPrimitiveType::Lines:
        ErrorHandling::Ensure(
            num_indices % 2 == 0,
            "Topology is {} but the number of indices is not a multiple of 2 ({} % 2 != 0)",
            topology,
            num_indices);
        break;

    case GlPrimitiveType::LineStrip:
        ErrorHandling::Ensure(
            num_indices > 1,
            "Topology is {} but the number of indices is less than 2 ({} < 2)",
            topology,
            num_indices);
        break;

    case GlPrimitiveType::Triangles:
        ErrorHandling::Ensure(
            num_indices % 3 == 0,
            "Topology is GL_TRIANGLES but the number of indices is not a multiple of 3 ({} % 3 != 0)",
            num_indices);
        break;

    case GlPrimitiveType::TriangleFan:
        ErrorHandling::Ensure(
            num_indices > 2,
            "Topology is GL_TRIANGLE_FAN but the number of indices is less than 3 ({})",
            num_indices);
        break;

    case klgl::GlPrimitiveType::Patches:
        break;

    default:
        ErrorHandling::ThrowWithMessage("Unknown topology with type {}", topology);
        break;
    }
}

void MeshOpenGL::Bind() const
{
    OpenGl::BindVertexArray(vao);
}

void MeshOpenGL::Draw() const
{
    constexpr auto allowed = ass::MakeEnumSet(
        GlPrimitiveType::Triangles,
        GlPrimitiveType::TriangleFan,
        GlPrimitiveType::Lines,
        GlPrimitiveType::LineStrip,
        GlPrimitiveType::Patches);
    assert(allowed.Contains(topology));
    OpenGl::DrawElements(topology, elements_count, GlIndexBufferElementType::UnsignedInt, nullptr);
}

void MeshOpenGL::DrawInstanced(const size_t num_instances)
{
    assert(topology == GlPrimitiveType::Triangles || topology == GlPrimitiveType::TriangleFan);
    OpenGl::DrawElementsInstanced(
        topology,
        elements_count,
        GlIndexBufferElementType::UnsignedInt,
        nullptr,
        num_instances);
}

void MeshOpenGL::BindAndDraw() const
{
    Bind();
    Draw();
}

}  // namespace klgl
