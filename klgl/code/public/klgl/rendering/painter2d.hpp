#pragma once

#include <memory>

#include "EverydayTools/Math/Matrix.hpp"
#include "klgl/shader/uniform_handle.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class Application;
class Shader;
class MeshOpenGL;

// This class allows you to paint simple figures every frame.
// It uses alpha channel but it will not sort entities by the distance to the camera automatically
class Painter2d
{
public:
    struct MeshVertex;
    struct Internal;

    explicit Painter2d(Application& app);
    ~Painter2d();

    struct Rect2d
    {
        Vec2f center;
        Vec2f size;
        Vec4f color = Vec4f(1, 1, 1, 1);
        float rotation_degrees = 0.f;
    };

    struct Circle2d
    {
        Vec2f center;
        Vec2f size;
        Vec4f color = Vec4f(1, 1, 1, 1);
        float rotation_degrees = 0.f;
    };

    void DrawRect(const Rect2d& rect);
    void DrawCircle(const Circle2d& circle);

private:
    Application* app_;
    std::unique_ptr<Shader> shader_;
    std::shared_ptr<MeshOpenGL> mesh_;
    UniformHandle u_type = klgl::UniformHandle("u_type");
    UniformHandle u_color_ = klgl::UniformHandle("u_color");
    UniformHandle u_transform_ = klgl::UniformHandle("u_transform");
};
}  // namespace klgl
