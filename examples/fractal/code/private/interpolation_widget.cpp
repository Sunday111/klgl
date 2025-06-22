#include "interpolation_widget.hpp"

#include "fractal_settings.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"

InterpolationWidget::InterpolationWidget(size_t num_colors)
{
    // Create quad mesh
    const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateQuadMesh();
    mesh_ =
        klgl::MeshOpenGL::MakeFromData(std::span{mesh_data.vertices}, std::span{mesh_data.indices}, mesh_data.topology);
    mesh_->Bind();

    // Vertex buffer attributes
    klgl::OpenGl::EnableVertexAttribArray(0);
    klgl::OpenGl::VertexAttribPointer(
        0,
        edt::Vec2f::Size(),
        klgl::GlVertexAttribComponentType::Float,
        false,
        sizeof(edt::Vec2f),
        nullptr);

    shader_ = std::make_unique<klgl::Shader>("interpolation_widget");
    shader_->SetDefineValue(def_colors_count, static_cast<int>(num_colors));
    shader_->Compile();

    u_color_table.resize(num_colors);
    for (size_t i = 0; i != u_color_table.size(); ++i)
    {
        auto uniform_name = klgl::Name(fmt::format("uColorTable[{}]", i));
        u_color_table[i] = shader_->GetUniform(uniform_name);
    }
}

InterpolationWidget::~InterpolationWidget() noexcept = default;

void InterpolationWidget::Render(const klgl::Viewport& viewport, const FractalSettings& settings)
{
    klgl::OpenGl::Viewport(
        static_cast<GLint>(viewport.position.x()),
        static_cast<GLint>(viewport.position.y()),
        static_cast<GLsizei>(viewport.size.x()),
        static_cast<GLsizei>(viewport.size.y()));

    shader_->Use();
    settings.ComputeColors(
        u_color_table.size(),
        [&](size_t i, const edt::Vec3f& color) { shader_->SetUniform(u_color_table[i], color); });

    shader_->SendUniforms();
    mesh_->BindAndDraw();
}
