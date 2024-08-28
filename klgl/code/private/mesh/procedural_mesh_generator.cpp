#include "klgl/mesh/procedural_mesh_generator.hpp"

#include "EverydayTools/Math/Constants.hpp"
#include "EverydayTools/Math/Math.hpp"

namespace klgl
{

GeneratedMeshData2d ProceduralMeshGenerator::GenerateQuadMesh()
{
    return {
        .vertices{
            {{1.0f, 1.0f}},    // right top
            {{1.0f, -1.0f}},   // right bottom
            {{-1.0f, -1.0f}},  // left bottom
            {{-1.0f, 1.0f}},   // left top
        },
        .texture_coordinates{
            {{1.f, 1.f}},
            {{1.f, 0.f}},
            {{0.f, 0.f}},
            {{0.f, 1.f}},
        },
        .indices = {0, 1, 3, 1, 2, 3},
        .topology = GlPrimitiveType::Triangles,
    };
}
GeneratedMeshData3d ProceduralMeshGenerator::GenerateCubeMesh()
{
    // This mesh has separate set of vertices for each face
    // to apply texture to each side
    GeneratedMeshData3d r{
        .vertices{
            // -z
            {{-1.0f, -1.0f, -1.0f}},
            {{-1.0f, +1.0f, -1.0f}},
            {{+1.0f, +1.0f, -1.0f}},
            {{+1.0f, -1.0f, -1.0f}},

            // +z
            {{-1.0f, -1.0f, +1.0f}},
            {{-1.0f, +1.0f, +1.0f}},
            {{+1.0f, +1.0f, +1.0f}},
            {{+1.0f, -1.0f, +1.0f}},

            // -x
            {{-1.0f, -1.0f, -1.0f}},
            {{-1.0f, -1.0f, +1.0f}},
            {{-1.0f, +1.0f, +1.0f}},
            {{-1.0f, +1.0f, -1.0f}},

            // +x
            {{+1.0f, -1.0f, -1.0f}},
            {{+1.0f, -1.0f, +1.0f}},
            {{+1.0f, +1.0f, +1.0f}},
            {{+1.0f, +1.0f, -1.0f}},

            // -y
            {{+1.0f, -1.0f, -1.0f}},
            {{+1.0f, -1.0f, +1.0f}},
            {{-1.0f, -1.0f, +1.0f}},
            {{-1.0f, -1.0f, -1.0f}},

            // +y
            {{+1.0f, +1.0f, -1.0f}},
            {{+1.0f, +1.0f, +1.0f}},
            {{-1.0f, +1.0f, +1.0f}},
            {{-1.0f, +1.0f, -1.0f}},
        },
        .texture_coordinates{},
        .indices = {0, 1,  2,  0,  2,  3,  4,  6,  5,  4,  7,  6,  8,  9,  10,
                    8, 10, 11, 12, 14, 13, 12, 15, 14, 16, 17, 18, 16, 18, 19},
        .topology = GlPrimitiveType::Triangles,
    };

    r.texture_coordinates.reserve(r.vertices.size());
    for (size_t i = 0; i != 6; ++i)
    {
        r.texture_coordinates.push_back({0.f, 0.f});
        r.texture_coordinates.push_back({0.f, 1.f});
        r.texture_coordinates.push_back({1.f, 1.f});
        r.texture_coordinates.push_back({1.f, 0.f});
    }

    return r;
}

std::optional<GeneratedMeshData2d> ProceduralMeshGenerator::GenerateCircleMesh(const size_t triangles_count)
{
    if (triangles_count < 3) return std::nullopt;

    std::vector<edt::Vec2f> vertices(triangles_count + 1);
    std::vector<uint32_t> indices(triangles_count + 2);

    const edt::Mat2f rotation =
        edt::Math::MakeRotationMatrix(2 * edt::kPi<float> / static_cast<float>(triangles_count));

    // circle center
    vertices[0] = {};

    // the first one is fixed and will be rotated by some fixed degree to make other points
    vertices[1] = {0.f, 1.f};

    for (size_t i = 2; i != vertices.size(); ++i)
    {
        vertices[i] = rotation.MatMul(vertices[i - 1]);
    }

    for (uint32_t i = 0; i != indices.size(); ++i)
    {
        indices[i] = i;
    }

    indices.back() = 1;

    std::vector<edt::Vec2f> tex_coord;
    tex_coord.reserve(vertices.size());
    std::ranges::copy(
        vertices | std::views::transform([&](const edt::Vec2f& v) { return (v + 1) / 2; }),
        std::back_inserter(tex_coord));

    return {{
        .vertices = std::move(vertices),
        .texture_coordinates = std::move(tex_coord),
        .indices = std::move(indices),
        .topology = GlPrimitiveType::TriangleFan,
    }};
}
}  // namespace klgl
