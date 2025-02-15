#pragma once

#include <memory>

#include "EverydayTools/Math/Matrix.hpp"

namespace klgl
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

// This class allows you to paint simple figures every frame.
// It uses alpha channel but it will not sort entities by the distance to the camera automatically
class Painter2d
{
public:
    class Impl;

    explicit Painter2d();
    ~Painter2d();

    struct Rect2d
    {
        Vec2f center;
        Vec2f size;
        Vec4u8 color = Vec4u8{} + 255;
        float rotation_degrees = 0.f;
    };

    struct Circle2d
    {
        Vec2f center;
        Vec2f size;
        Vec4u8 color = Vec4u8{} + 255;
        float rotation_degrees = 0.f;
    };

    struct Triangle2d
    {
        Vec2f a{};
        Vec2f b{};
        Vec2f c{};
        Vec4u8 color = Vec4u8{} + 255;
    };

    struct Line2d
    {
        Vec2f a{};
        Vec2f b{};
        Vec4u8 color = Vec4u8{} + 255;
        float width = 0.01f;
    };

    struct LineWidth
    {
        float inner = 0;
        float outer = 0;
    };

    void BeginDraw();
    void EndDraw();

    void RectLines(const Rect2d& rect, LineWidth line_width);
    void TriangleLines(const Triangle2d& triangle, LineWidth line_width);

    void FillRect(const Rect2d& rect);
    void FillCircle(const Circle2d& circle);
    void FillTriangle(const Triangle2d& triangle);
    void DrawLine(const Line2d& line);

    void SetViewMatrix(const Mat3f& view_matrix);

private:
    std::unique_ptr<Impl> self;
};
}  // namespace klgl
