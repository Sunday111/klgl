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

    edt::Mat3f world_to_camera{};
    edt::Mat3f camera_to_view{};
    edt::Mat3f world_to_view{};

    edt::Mat3f view_to_camera{};
    edt::Mat3f camera_to_world{};
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
    edt::Vec2f half_camera_extent{1, 1};
    switch (aspect_ratio_policy)
    {
    case AspectRatioPolicy::Stretch:
        half_camera_extent = edt::Vec2f{1, 1};
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
    world_to_camera = edt::Math::TranslationMatrix(-camera.eye);
    camera_to_view = edt::Math::ScaleMatrix(1.f / half_camera_extent);
    world_to_view = camera_to_view.MatMul(world_to_camera);

    // Backwards
    view_to_camera = edt::Math::ScaleMatrix(half_camera_extent);
    camera_to_world = edt::Math::TranslationMatrix(camera.eye);
    view_to_world = camera_to_world.MatMul(view_to_camera);

    screen_to_view = (edt::Math::TranslationMatrix(edt::Vec2f{} - 1).MatMul(edt::Math::ScaleMatrix(2 / viewport.size)))
                         .MatMul(edt::Math::TranslationMatrix(-viewport.position));

    screen_to_world = view_to_world.MatMul(screen_to_view);
}

}  // namespace klgl
