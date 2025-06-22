#include "simple_gpu_renderer.hpp"

#include "fractal_settings.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"

SimpleGpuRenderer::SimpleGpuRenderer()
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

    fractal_shader_ = std::make_unique<klgl::Shader>("fractal");

    max_iterations = static_cast<size_t>(fractal_shader_->GetDefineValue<int>(def_max_iterations));
    u_color_table.resize(max_iterations + 1);

    for (size_t i = 0; i != u_color_table.size(); ++i)
    {
        auto uniform_name = klgl::Name(fmt::format("uColorTable[{}]", i));
        u_color_table[i] = fractal_shader_->GetUniform(uniform_name);
    }
}

SimpleGpuRenderer::~SimpleGpuRenderer() noexcept = default;

void SimpleGpuRenderer::Render(const FractalSettings& settings)
{
    klgl::OpenGl::Viewport(
        static_cast<GLint>(settings.viewport.position.x()),
        static_cast<GLint>(settings.viewport.position.y()),
        static_cast<GLsizei>(settings.viewport.size.x()),
        static_cast<GLsizei>(settings.viewport.size.y()));
    fractal_shader_->Use();

    render_transforms_.Update(settings.camera, settings.viewport);

    fractal_shader_->SetUniform(u_screen_to_world_, render_transforms_.screen_to_world.Transposed());
    fractal_shader_->SetUniform(u_julia_constant, settings.MakeJuliaConstant());
    if (u_resolution_) fractal_shader_->SetUniform(*u_resolution_, settings.viewport.size);
    if (u_time_) fractal_shader_->SetUniform(*u_time_, settings.time);

    fractal_shader_->SendUniforms();
    mesh_->BindAndDraw();
}

void SimpleGpuRenderer::ApplySettings(const FractalSettings& settings)
{
    fractal_shader_->SetDefineValue<int>(def_inside_out_space, settings.inside_out_space ? 1 : 0);
    fractal_shader_->SetDefineValue<int>(def_color_mode, settings.color_mode);
    fractal_shader_->Compile();

    u_resolution_ = fractal_shader_->FindUniform(klgl::Name("u_resolution"));
    u_time_ = fractal_shader_->FindUniform(klgl::Name("u_time"));

    settings.ComputeColors(
        u_color_table.size(),
        [&](size_t index, const edt::Vec3f& color) { fractal_shader_->SetUniform(u_color_table[index], color); });
}
