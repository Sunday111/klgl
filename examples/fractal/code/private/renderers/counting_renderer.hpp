#pragma once

#include <klgl/opengl/identifiers.hpp>
#include <klgl/opengl/object.hpp>
#include <memory>
#include <vector>

#include "fractal_renderer.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/shader/define_handle.hpp"
#include "klgl/shader/uniform_handle.hpp"

namespace klgl
{
class Shader;
struct MeshOpenGL;
}  // namespace klgl

class CountingRenderer : public FractalRenderer
{
public:
    explicit CountingRenderer(size_t max_iterations);
    ~CountingRenderer() noexcept override;

    void Render(const FractalSettings&) override;
    void ApplySettings(const FractalSettings&) override;

    size_t max_iterations{};

    klgl::DefineHandle def_compute_max_iterations{klgl::Name("MAX_ITERATIONS")};
    klgl::DefineHandle def_compute_inside_out_space{klgl::Name("INSIDE_OUT_SPACE")};
    klgl::UniformHandle u_compute_screen_to_world_{"u_screen_to_world"};
    std::optional<klgl::UniformHandle> u_compute_world_to_screen_;
    klgl::UniformHandle u_compute_resolution_{"u_resolution"};
    klgl::UniformHandle u_compute_julia_constant_{"u_julia_constant"};

    klgl::DefineHandle def_draw_max_iterations{klgl::Name("MAX_ITERATIONS")};
    klgl::UniformHandle u_draw_resolution_{"u_resolution"};
    std::vector<klgl::UniformHandle> u_color_table;

    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    klgl::RenderTransforms2d render_transforms_;

    std::shared_ptr<klgl::Shader> draw_shader_;
    std::shared_ptr<klgl::Shader> compute_shader_;

    klgl::GlObject<klgl::GlVertexArrayId> counters_vao_;
    klgl::GlObject<klgl::GlBufferId> counters_buffer_;
    size_t current_counters_vao_size_ = 0;
};
