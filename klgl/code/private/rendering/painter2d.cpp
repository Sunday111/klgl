#include "klgl/rendering/painter2d.hpp"

#include "EverydayTools/Math/Math.hpp"
#include "klgl/application.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/template/type_to_gl_type.hpp"

namespace klgl
{

class Painter2d::Impl
{
public:
    Application* app_ = nullptr;
    std::unique_ptr<Shader> shader_;
    std::shared_ptr<MeshOpenGL> mesh_;
    UniformHandle u_type = klgl::UniformHandle("u_type");
    UniformHandle u_color_ = klgl::UniformHandle("u_color");
    UniformHandle u_transform_ = klgl::UniformHandle("u_transform");
};

Painter2d::Painter2d(Application& app) : self(std::make_unique<Impl>())
{
    self->app_ = &app;
    self->shader_ = std::make_unique<klgl::Shader>("klgl/painter2d.shader.json");
    // Create quad mesh
    const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateQuadMesh();

    self->mesh_ =
        klgl::MeshOpenGL::MakeFromData(std::span{mesh_data.vertices}, std::span{mesh_data.indices}, mesh_data.topology);
    self->mesh_->Bind();

    // Vertex buffer attributes
    OpenGl::EnableVertexAttribArray(0);
    klgl::OpenGl::EnableVertexAttribArray(0);
    using GlTypeTraits = klgl::TypeToGlType<edt::Vec2f>;
    klgl::OpenGl::VertexAttribPointer(
        0,
        GlTypeTraits::Size,
        GlTypeTraits::AttribComponentType,
        false,
        sizeof(edt::Vec2f),
        nullptr);  // NOLINT
}

Painter2d::~Painter2d() = default;

void Painter2d::DrawRect(const Rect2d& rect)
{
    self->shader_->Use();

    klgl::OpenGl::EnableBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto m = edt::Math::ScaleMatrix(rect.size / 2);
    if (rect.rotation_degrees != 0.f)
    {
        const float radians = edt::Math::DegToRad(rect.rotation_degrees);
        m = edt::Math::RotationMatrix2d(radians).MatMul(m);
    }

    m = edt::Math::TranslationMatrix(rect.center).MatMul(m);
    self->shader_->SetUniform(self->u_type, 0);
    self->shader_->SetUniform(self->u_transform_, m.Transposed());
    self->shader_->SetUniform(self->u_color_, rect.color);

    self->shader_->SendUniforms();
    self->mesh_->BindAndDraw();
}

void Painter2d::DrawCircle(const Circle2d& circle)
{
    self->shader_->Use();

    klgl::OpenGl::EnableBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto m = edt::Math::ScaleMatrix(circle.size / 2.f);
    if (circle.rotation_degrees != 0.f)
    {
        const float radians = edt::Math::DegToRad(circle.rotation_degrees);
        m = edt::Math::RotationMatrix2d(radians).MatMul(m);
    }

    m = edt::Math::TranslationMatrix(circle.center).MatMul(m);
    self->shader_->SetUniform(self->u_type, 1);
    self->shader_->SetUniform(self->u_transform_, m.Transposed());
    self->shader_->SetUniform(self->u_color_, circle.color);

    self->shader_->SendUniforms();
    self->mesh_->BindAndDraw();
}

void Painter2d::DrawTriangle(const Triangle2d& triangle)
{
    self->shader_->Use();

    klgl::OpenGl::EnableBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // The transformation below is an inlined version of the following algorithm:
    // 1. Translate by 1 so that bottom left corner in screen space (point A) becomes 0, 0
    // 2. Transform the quad space so that x axis becomes AB and y axis becomes AC
    // 3. Move point a to bottom left corner of screen space
    // Mat3f basis;
    // basis.SetColumn(0, Vec3f{i, 0});
    // basis.SetColumn(1, Vec3f{j, 0});
    // basis.SetColumn(2, Vec3f{0, 0, 1});
    // m = edt::Math::TranslationMatrix(triangle.a).MatMul(basis.MatMul(edt::Math::TranslationMatrix(Vec2f{} + 1)));

    const Vec2f i = (triangle.b - triangle.a) / 2;
    const Vec2f j = (triangle.c - triangle.a) / 2;
    const Vec2f t = (triangle.b + triangle.c) / 2;

    Mat3f m;
    m.SetColumn(0, Vec3f{i, 0});
    m.SetColumn(1, Vec3f{j, 0});
    m.SetColumn(2, Vec3f(t, 1));

    self->shader_->SetUniform(self->u_type, 2);
    self->shader_->SetUniform(self->u_transform_, m.Transposed());
    self->shader_->SetUniform(self->u_color_, triangle.color);

    self->shader_->SendUniforms();
    self->mesh_->BindAndDraw();
}

}  // namespace klgl
