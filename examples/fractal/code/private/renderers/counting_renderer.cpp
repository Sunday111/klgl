#include "counting_renderer.hpp"

#include <klgl/opengl/detail/maps/to_gl_value/buffer_type.hpp>
#include <klgl/opengl/vertex_attribute_helper.hpp>

#include "fractal_settings.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"

CountingRenderer::CountingRenderer(size_t max_iterations_) : max_iterations(max_iterations_)
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

    compute_shader_ = std::make_unique<klgl::Shader>("counting_fractal_compute");
    counters_vao_ = klgl::GlObject<klgl::GlVertexArrayId>::CreateFrom(klgl::OpenGl::GenVertexArray());

    draw_shader_ = std::make_unique<klgl::Shader>("counting_fractal");

    u_color_table.resize(max_iterations + 1);

    for (size_t i = 0; i != u_color_table.size(); ++i)
    {
        auto uniform_name = klgl::Name(fmt::format("u_color_table[{}]", i));
        u_color_table[i] = draw_shader_->GetUniform(uniform_name);
    }
}

CountingRenderer::~CountingRenderer() noexcept = default;

void CountingRenderer::Render(const FractalSettings& settings)
{
    klgl::OpenGl::Viewport(
        static_cast<GLint>(settings.viewport.position.x()),
        static_cast<GLint>(settings.viewport.position.y()),
        static_cast<GLsizei>(settings.viewport.size.x()),
        static_cast<GLsizei>(settings.viewport.size.y()));

    render_transforms_.Update(settings.camera, settings.viewport);

    compute_shader_->Use();
    compute_shader_->SetUniform(u_compute_screen_to_world_, render_transforms_.screen_to_world.Transposed());
    if (u_compute_world_to_screen_)
    {
        compute_shader_->SetUniform(*u_compute_world_to_screen_, render_transforms_.world_to_screen.Transposed());
    }
    compute_shader_->SetUniform(u_compute_resolution_, settings.viewport.size);
    compute_shader_->SetUniform(u_compute_julia_constant_, settings.MakeJuliaConstant());
    compute_shader_->SendUniforms();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, counters_buffer_.GetId().GetValue());
    klgl::OpenGl::BindBuffer(klgl::GlBufferType::ShaderStorage, counters_buffer_);
    glClearBufferData(
        klgl::ToGlValue(klgl::GlBufferType::ShaderStorage),
        GL_R32UI,  // must match your buffer format
        GL_RED_INTEGER,
        GL_UNSIGNED_INT,
        nullptr  // NULL = zero-filled
    );

    int groupSize = 16;
    auto resolution = settings.viewport.size.Cast<int>();
    glDispatchCompute((resolution.x() + groupSize - 1) / groupSize, (resolution.y() + groupSize - 1) / groupSize, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // {  // Bind the buffer for reading

    //     // Map the buffer to client memory
    //     void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    //     assert(ptr);

    //     // Interpret as uint32_t
    //     uint32_t* data = static_cast<uint32_t*>(ptr);

    //     // Count frequencies
    //     std::unordered_map<uint32_t, size_t> frequencies;
    //     uint32_t sum = 0;
    //     for (size_t i = 0; i < current_counters_vao_size_; ++i)
    //     {
    //         ++frequencies[data[i]];  // NOLINT
    //         sum += data[i];          // NOLINT
    //     }

    //     glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    //     // Optional: sort by value or frequency
    //     std::vector<std::pair<uint32_t, size_t>> sorted(frequencies.begin(), frequencies.end());
    //     std::sort(sorted.begin(), sorted.end());

    //     // Print results
    //     for (const auto& [val, freq] : sorted)
    //     {
    //         fmt::println("{}, {}", val, freq);
    //     }
    //     fmt::println("{}x{}", resolution.x(), resolution.y());
    //     fmt::println("sum: {}", sum);
    //     fmt::println("empty: {}", frequencies[0]);
    // }

    draw_shader_->Use();
    draw_shader_->SetUniform(u_draw_resolution_, settings.viewport.size);
    draw_shader_->SendUniforms();
    mesh_->BindAndDraw();
}

void CountingRenderer::ApplySettings(const FractalSettings& settings)
{
    compute_shader_->SetDefineValue(def_compute_max_iterations, static_cast<int>(max_iterations));
    compute_shader_->SetDefineValue(def_compute_inside_out_space, settings.inside_out_space ? 1 : 0);
    compute_shader_->Compile();

    u_compute_world_to_screen_ = compute_shader_->FindUniform(klgl::Name{"u_world_to_screen_"});

    klgl::OpenGl::BindVertexArray(counters_vao_);
    auto resolution = settings.viewport.size.Cast<size_t>();
    size_t num_pixels = resolution.x() * resolution.y();
    if (current_counters_vao_size_ != num_pixels)
    {
        counters_buffer_ = klgl::GlObject<klgl::GlBufferId>::CreateFrom(klgl::OpenGl::GenBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, counters_buffer_.GetId().GetValue());
        std::vector<uint32_t> counters(num_pixels, 0);
        klgl::OpenGl::BufferData(klgl::GlBufferType::ShaderStorage, std::span{counters}, klgl::GlUsage::DynamicDraw);
        current_counters_vao_size_ = num_pixels;
    }

    draw_shader_->SetDefineValue(def_draw_max_iterations, static_cast<int>(max_iterations));
    draw_shader_->Compile();
    settings.ComputeColors(
        u_color_table.size(),
        [&](size_t index, const edt::Vec3f& color) { draw_shader_->SetUniform(u_color_table[index], color); });
}
