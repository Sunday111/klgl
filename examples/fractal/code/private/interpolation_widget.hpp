#pragma once

#include <klgl/camera/camera_2d.hpp>
#include <memory>
#include <vector>

#include "klgl/shader/define_handle.hpp"
#include "klgl/shader/uniform_handle.hpp"

namespace klgl
{
class Shader;
struct MeshOpenGL;
}  // namespace klgl

class FractalSettings;

class InterpolationWidget
{
public:
    explicit InterpolationWidget(size_t num_colors);
    ~InterpolationWidget() noexcept;

    void Render(const klgl::Viewport& viewport, const FractalSettings& settings);

    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    std::vector<klgl::UniformHandle> u_color_table;
    klgl::DefineHandle def_colors_count{klgl::Name("COLORS_COUNT")};
};
