#include "klgl/rendering/painter2d.hpp"

#include "EverydayTools/Math/Math.hpp"
#include "klgl/application.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/template/register_attribute.hpp"

namespace klgl
{

struct Painter2d::MeshVertex
{
    edt::Vec2f position{};
    edt::Vec2f texture_coordinates{};
};

struct Painter2d::Internal
{
};

Painter2d::Painter2d(Application& app) : app_(&app)
{
    shader_ = std::make_unique<klgl::Shader>("klgl/painter2d.shader.json");
    // Create quad mesh
    const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateQuadMesh();

    std::vector<MeshVertex> vertices;
    vertices.reserve(mesh_data.vertices.size());
    for (size_t i = 0; i != mesh_data.vertices.size(); ++i)
    {
        vertices.emplace_back(MeshVertex{
            .position = mesh_data.vertices[i],
            .texture_coordinates = mesh_data.texture_coordinates[i],
        });
    }

    mesh_ = klgl::MeshOpenGL::MakeFromData(std::span{vertices}, std::span{mesh_data.indices}, mesh_data.topology);
    mesh_->Bind();

    // Vertex buffer attributes
    OpenGl::EnableVertexAttribArray(0);
    RegisterAttribute<&MeshVertex::position>(0, false);
    RegisterAttribute<&MeshVertex::texture_coordinates>(1, false);
}

Painter2d::~Painter2d() = default;

void Painter2d::DrawRect(const Rect2d& rect)
{
    shader_->Use();

    auto m = edt::Math::ScaleMatrix(rect.size);
    if (rect.rotation_degrees != 0.f)
    {
        const float radians = edt::Math::DegToRad(rect.rotation_degrees);
        m = edt::Math::RotationMatrix2d(radians).MatMul(m);
    }

    m = edt::Math::TranslationMatrix(rect.center).MatMul(m);
    shader_->SetUniform(u_type, 0);
    shader_->SetUniform(u_transform_, m.Transposed());
    shader_->SetUniform(u_color_, rect.color);

    shader_->SendUniforms();
    mesh_->BindAndDraw();
}

void Painter2d::DrawCircle(const Circle2d& circle)
{
    shader_->Use();

    klgl::OpenGl::EnableBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto m = edt::Math::ScaleMatrix(circle.size);
    if (circle.rotation_degrees != 0.f)
    {
        const float radians = edt::Math::DegToRad(circle.rotation_degrees);
        m = edt::Math::RotationMatrix2d(radians).MatMul(m);
    }

    m = edt::Math::TranslationMatrix(circle.center).MatMul(m);
    shader_->SetUniform(u_type, 1);
    shader_->SetUniform(u_transform_, m.Transposed());
    shader_->SetUniform(u_color_, circle.color);

    shader_->SendUniforms();
    mesh_->BindAndDraw();
}

}  // namespace klgl
