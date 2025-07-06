#include "klgl/rendering/curve_renderer_2d.hpp"

#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/template/register_attribute.hpp"

namespace klgl
{
CurveRenderer2d::CurveRenderer2d() = default;
CurveRenderer2d::~CurveRenderer2d() = default;

void CurveRenderer2d::Init()
{
    shader_ = std::make_unique<klgl::Shader>("klgl/curve2d");
}

void CurveRenderer2d::GenIndices(const uint32_t n)
{
    klgl::ErrorHandling::Ensure(n > 1, "Need at least 2 points for a curve");

    indices.resize((n - 1) * 4);

    if (n == 2)
    {
        // Just two points. Repeat the firt and the last index
        indices = {0, 0, 1, 1};
        return;
    }

    // At least 3 points.

    // Duplicate the first point
    uint32_t j = 0;
    indices[j++] = 0;
    indices[j++] = 0;
    indices[j++] = 1;
    indices[j++] = 2;

    const auto num_inner = (n - 3) * 4;
    for (uint32_t i = 0; i != num_inner; ++i)
    {
        indices[j++] = i / 4 + i % 4;
    }

    // Duplicate the last point
    indices[j++] = n - 3;
    indices[j++] = n - 2;
    indices[j++] = n - 1;
    indices[j++] = n - 1;

    klgl::ErrorHandling::Ensure(j == indices.size(), "Internal logic err");
}

void CurveRenderer2d::SetPoints(std::span<const ControlPoint> points)
{
    if (!shader_) Init();

    GenIndices(static_cast<uint32_t>(points.size()));

    shader_->Use();
    mesh_ = klgl::MeshOpenGL::MakeFromData(std::span{points}, std::span{indices}, klgl::GlPrimitiveType::Patches);
    mesh_->Bind();

    klgl::RegisterAttribute<&ControlPoint::position>(
        shader_->GetInfo().VerifyAndGetVertexAttributeLocation<Vec2f>("a_vertex"));
    klgl::RegisterAttribute<&ControlPoint::color>(
        shader_->GetInfo().VerifyAndGetVertexAttributeLocation<Vec4f>("a_color"));
}

void CurveRenderer2d::Draw(Vec2f viewport_size, const Mat3f& world_to_view)
{
    mesh_->Bind();
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glLineWidth(1.f);
    shader_->Use();
    shader_->SetUniform(u_transform_, world_to_view.Transposed());
    shader_->SetUniform(u_viewport_size_, viewport_size);
    shader_->SetUniform(u_thickness_, thickness_);
    shader_->SendUniforms();

    mesh_->BindAndDraw();
}
}  // namespace klgl
