#pragma once

#include <EverydayTools/Math/FloatRange.hpp>
#include <EverydayTools/Math/Math.hpp>

namespace klgl
{

class Camera2d;
class Viewport;

enum class AspectRatioPolicy : uint8_t
{
    Stretch,
    ShrinkToFit,
    GrowToFill
};

class RenderTransforms2d
{
public:
    constexpr void Update(
        const Camera2d camera,
        const Viewport& vierport,
        AspectRatioPolicy aspect_ratio_policy = AspectRatioPolicy::ShrinkToFit);

    edt::Mat3f world_to_view{};
    edt::Mat3f view_to_screen{};
    edt::Mat3f world_to_screen{};

    edt::Mat3f view_to_world{};
    edt::Mat3f screen_to_view{};
    edt::Mat3f screen_to_world{};
};

class Viewport
{
public:
    [[nodiscard]] constexpr float GetAspect() const { return size.x() / size.y(); }

    constexpr void MatchWindowSize(const edt::Vec2f& window_size)
    {
        position = {};
        size = window_size;
    }

    [[nodiscard]] constexpr bool operator==(const Viewport& rhs) const noexcept
    {
        return position == rhs.position && size == rhs.size;
    }

    [[nodiscard]] constexpr bool operator!=(const Viewport& rhs) const noexcept { return !(*this == rhs); }

    void UseInOpenGL();

    edt::Vec2f position;
    edt::Vec2f size;
};

class Camera2d
{
public:
    float zoom = 1.f;
    edt::Vec2f eye{};
};

constexpr void
RenderTransforms2d::Update(const Camera2d camera, const Viewport& viewport, AspectRatioPolicy aspect_ratio_policy)
{
    using edt::Math, edt::Vec2f;

    edt::Vec2f half_camera_extent{1, 1};
    switch (aspect_ratio_policy)
    {
    case AspectRatioPolicy::Stretch:
        half_camera_extent = Vec2f{1, 1};
        break;
    case AspectRatioPolicy::ShrinkToFit:
        half_camera_extent = viewport.size / viewport.size.Min();
        break;
    case AspectRatioPolicy::GrowToFill:
        half_camera_extent = viewport.size / viewport.size.Max();
        break;
    }

    half_camera_extent /= camera.zoom;

    // Forward
    world_to_view = Math::MatMul(Math::ScaleMatrix(1.f / half_camera_extent), Math::TranslationMatrix(-camera.eye));
    view_to_screen = Math::MatMul(
        Math::TranslationMatrix(viewport.position),
        Math::ScaleMatrix(viewport.size / 2),
        Math::TranslationMatrix(Vec2f{} + 1));
    world_to_screen = Math::MatMul(view_to_screen, world_to_view);

    // Backwards
    view_to_world = Math::MatMul(Math::TranslationMatrix(camera.eye), Math::ScaleMatrix(half_camera_extent));
    screen_to_view = Math::MatMul(
        Math::TranslationMatrix(Vec2f{} - 1),
        Math::ScaleMatrix(2 / viewport.size),
        Math::TranslationMatrix(-viewport.position));
    screen_to_world = Math::MatMul(view_to_world, screen_to_view);
}

}  // namespace klgl
