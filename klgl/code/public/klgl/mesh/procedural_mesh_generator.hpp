#pragma once

#include "klgl/mesh/mesh_data.hpp"
namespace klgl
{

class ProceduralMeshGenerator
{
public:
    static std::optional<MeshData> GenerateCircleMesh(const size_t triangles_count);
    static MeshData GenerateQuadMesh();
};
}  // namespace klgl
