#include "klgl/rendering/curve_renderer_2d.hpp"

#include <algorithm>

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

    // Each Catmull-Rom patch needs four control points. Keep one additional
    // point on either side so the tessellation stage can evaluate the exact
    // adjacent generated segment at patch boundaries as well.
    indices.resize((n - 1) * 6);
    size_t output_index = 0;
    for (uint32_t segment = 0; segment != n - 1; ++segment)
    {
        for (int32_t offset = -2; offset <= 3; ++offset)
        {
            const int32_t point = static_cast<int32_t>(segment) + offset;
            indices[output_index++] =
                static_cast<uint32_t>(std::clamp(point, int32_t{0}, static_cast<int32_t>(n - 1)));
        }
    }
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
    if (!shader_) return;
    mesh_->Bind();
    glPatchParameteri(GL_PATCH_VERTICES, 6);
    glLineWidth(1.f);
    shader_->Use();
    shader_->SetUniform(u_transform_, world_to_view.Transposed());
    shader_->SetUniform(u_viewport_size_, viewport_size);
    shader_->SetUniform(u_thickness_, thickness_);
    shader_->SetUniform(u_segment_pixel_length_, segment_pixel_length_);
    shader_->SendUniforms();

    mesh_->BindAndDraw();
}
}  // namespace klgl
