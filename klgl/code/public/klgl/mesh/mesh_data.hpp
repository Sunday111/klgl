#pragma once

#include <memory>
#include <span>
#include <vector>

#include "klgl/opengl/gl_api.hpp"

namespace klgl
{

class MeshData
{
public:
    std::vector<Vec2f> vertices;
    std::vector<Vec2f> texture_coordinates;
    std::vector<uint32_t> indices;
    GlPrimitiveType topology = GlPrimitiveType::Triangles;
};

class MeshOpenGL
{
public:
    static void ValidateIndicesCountForTopology(const GlPrimitiveType topology, const size_t num_indices);

    template <typename Vertex, size_t VE = std::dynamic_extent, size_t IE = std::dynamic_extent>
    static std::unique_ptr<MeshOpenGL>
    MakeFromData(std::span<Vertex, VE> vertices, std::span<const uint32_t, IE> indices, GlPrimitiveType topology)
    {
        ValidateIndicesCountForTopology(topology, indices.size());

        auto mesh = std::make_unique<MeshOpenGL>();

        mesh->vao = OpenGl::GenVertexArray();
        mesh->vbo = OpenGl::GenBuffer();
        mesh->ebo = OpenGl::GenBuffer();

        mesh->Bind();
        OpenGl::BindBuffer(GlBufferType::Array, mesh->vbo);
        OpenGl::BufferData(GlBufferType::Array, vertices, GlUsage::StaticDraw);
        OpenGl::BindBuffer(GlBufferType::ElementArray, mesh->ebo);
        OpenGl::BufferData(GlBufferType::ElementArray, std::span{indices}, GlUsage::StaticDraw);

        mesh->elements_count = indices.size();
        mesh->topology = topology;

        return mesh;
    }

    ~MeshOpenGL();

    void Bind() const;
    void BindAndDraw() const;
    void Draw() const;
    void DrawInstanced(const size_t num_instances);

    GlVertexArrayId vao{};
    GlBufferId vbo{};
    GlBufferId ebo{};
    GlPrimitiveType topology = GlPrimitiveType::Triangles;
    size_t elements_count = 0;
};

}  // namespace klgl
