#pragma once

#include <memory>
#include <vector>

#include "fractal_renderer.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/shader/uniform_handle.hpp"

namespace klgl
{
class Shader;
class Texture;
struct MeshOpenGL;
}  // namespace klgl

class SimpleCpuRenderer : public FractalRenderer
{
public:
    explicit SimpleCpuRenderer(size_t max_iterations);
    ~SimpleCpuRenderer() noexcept override;

    void Render(const FractalSettings&) override;
    void ApplySettings(const FractalSettings&) override;

    size_t max_iterations{};

    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    klgl::RenderTransforms2d render_transforms_;
    std::unique_ptr<klgl::Texture> texture_;

    std::shared_ptr<klgl::Shader> shader_;
    klgl::UniformHandle u_texture_{"u_texture"};
    size_t a_vertex_{};
    size_t a_tex_coord_{};
    std::vector<edt::Vec3f> pallette;
    std::vector<edt::Vec3f> image_buffer_;
};
