#pragma once

#include <memory>
#include <span>

#include "klgl/opengl/object.hpp"

namespace klgl
{

class MeshOpenGL
{
public:
    static void ValidateIndicesCountForTopology(const GlPrimitiveType topology, const size_t num_indices);

    template <typename Vertex, typename Index, size_t VE = std::dynamic_extent, size_t IE = std::dynamic_extent>
        requires std::same_as<std::remove_const_t<Index>, uint32_t>
    static std::unique_ptr<MeshOpenGL>
    MakeFromData(std::span<Vertex, VE> vertices, std::span<Index, IE> indices, GlPrimitiveType topology)
    {
        ValidateIndicesCountForTopology(topology, indices.size());

        auto mesh = std::make_unique<MeshOpenGL>();

        mesh->vao = GlObject<GlVertexArrayId>::CreateFrom(OpenGl::GenVertexArray());
        mesh->vbo = GlObject<GlBufferId>::CreateFrom(OpenGl::GenBuffer());
        mesh->ebo = GlObject<GlBufferId>::CreateFrom(OpenGl::GenBuffer());

        mesh->Bind();
        OpenGl::BindBuffer(GlBufferType::Array, mesh->vbo);
        OpenGl::BufferData(GlBufferType::Array, vertices, GlUsage::StaticDraw);
        OpenGl::BindBuffer(GlBufferType::ElementArray, mesh->ebo);
        OpenGl::BufferData(GlBufferType::ElementArray, indices, GlUsage::StaticDraw);

        mesh->elements_count = indices.size();
        mesh->topology = topology;

        return mesh;
    }

    void Bind() const;
    void BindAndDraw() const;
    void Draw() const;
    void DrawInstanced(const size_t num_instances);

    GlObject<GlVertexArrayId> vao{};
    GlObject<GlBufferId> vbo{};
    GlObject<GlBufferId> ebo{};
    GlPrimitiveType topology = GlPrimitiveType::Triangles;
    size_t elements_count = 0;
};

}  // namespace klgl
