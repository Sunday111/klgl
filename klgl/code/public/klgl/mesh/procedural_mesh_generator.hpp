#pragma once

#include <vector>

#include "EverydayTools/Math/Matrix.hpp"
#include "klgl/opengl/enums.hpp"

namespace klgl
{

class GeneratedMeshData2d
{
public:
    std::vector<edt::Vec2f> vertices;
    std::vector<edt::Vec2f> texture_coordinates;
    std::vector<uint32_t> indices;
    GlPrimitiveType topology = GlPrimitiveType::Triangles;
};

class GeneratedMeshData3d
{
public:
    std::vector<edt::Vec3f> vertices;
    std::vector<edt::Vec3f> normals;
    std::vector<edt::Vec2f> texture_coordinates;
    std::vector<uint32_t> indices;
    GlPrimitiveType topology = GlPrimitiveType::Triangles;
};

class ProceduralMeshGenerator
{
public:
    static std::optional<GeneratedMeshData2d> GenerateCircleMesh(const size_t triangles_count);
    static GeneratedMeshData2d GenerateQuadMesh();
    static GeneratedMeshData3d GenerateCubeMesh();
};
}  // namespace klgl
