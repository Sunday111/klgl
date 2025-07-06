#pragma once

#include <memory>
#include <vector>

#include "EverydayTools/math/Matrix.hpp"
#include "klgl/shader/uniform_handle.hpp"

namespace klgl
{

class Shader;
class MeshOpenGL;

class CurveRenderer2d
{
public:
    struct ControlPoint
    {
        alignas(edt::Vec4f) edt::Vec2f position{};
        edt::Vec4f color{};
    };

    CurveRenderer2d();
    ~CurveRenderer2d();

    void Init();
    void GenIndices(const uint32_t n);
    void SetPoints(std::span<const ControlPoint> points);
    void Draw(edt::Vec2f viewport_size, const edt::Mat3f& world_to_view);

    float thickness_ = 5.f;
    std::vector<uint32_t> indices;
    std::unique_ptr<Shader> shader_;
    std::shared_ptr<MeshOpenGL> mesh_;
    UniformHandle u_transform_ = UniformHandle("u_transform");
    UniformHandle u_viewport_size_ = UniformHandle("u_viewport_size");
    UniformHandle u_thickness_ = UniformHandle("u_thickness");
};
}  // namespace klgl
