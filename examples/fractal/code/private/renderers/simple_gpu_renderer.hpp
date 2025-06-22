#pragma once

#include <memory>
#include <optional>
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

class SimpleGpuRenderer : public FractalRenderer
{
public:
    explicit SimpleGpuRenderer(size_t max_iterations);
    ~SimpleGpuRenderer() noexcept override;

    void Render(const FractalSettings&) override;
    void ApplySettings(const FractalSettings&) override;

    klgl::DefineHandle def_inside_out_space{klgl::Name("INSIDE_OUT_SPACE")};
    klgl::DefineHandle def_max_iterations{klgl::Name("MAX_ITERATIONS")};
    klgl::DefineHandle def_color_mode{klgl::Name("COLOR_MODE")};
    std::optional<klgl::UniformHandle> u_time_;
    std::optional<klgl::UniformHandle> u_resolution_;
    klgl::UniformHandle u_screen_to_world_ = klgl::UniformHandle("u_screen_to_world");
    klgl::UniformHandle u_julia_constant = klgl::UniformHandle("u_julia_constant");
    std::vector<klgl::UniformHandle> u_color_table;

    size_t max_iterations{};

    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    klgl::RenderTransforms2d render_transforms_;

    std::shared_ptr<klgl::Shader> fractal_shader_;
};
