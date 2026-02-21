#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <algorithm>
#include <klgl/camera/camera_2d.hpp>
#include <klgl/events/event_listener_interface.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <set>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/rendering/painter2d.hpp"
#include "klgl/window.hpp"

namespace klgl::painter2d_example
{

template <typename T>
struct TaggedIdHasher
{
    [[nodiscard]] constexpr size_t operator()(const T& v) const noexcept
    {
        return std::hash<typename T::Repr>{}(v.GetValue());
    }
};

namespace tags
{
struct ParticleIdTag
{
};
}  // namespace tags

using ParticleId = edt::TaggedIdentifier<tags::ParticleIdTag, uint32_t>;

// [[nodiscard]] static constexpr ImVec2 ToImVec(Vec2f v) noexcept
// {
//     return ImVec2(v.x(), v.y());
// }

struct GridRegion
{
    Vec2i pos;
    static constexpr edt::Vec2u16 kSize{256, 256};

    std::array<ass::FixedBitset<kSize.x()>, kSize.y()> bits{};
    std::array<std::array<ParticleId, kSize.x()>, kSize.y()> ids;
};

using RegionId = Vec2i;

[[nodiscard]] static constexpr RegionId ToRegionId(Vec2i coord) noexcept
{
    Vec2i neg = Vec2i{coord.x() < 0, coord.y() < 0};
    return ((coord + neg) / GridRegion::kSize.Cast<int32_t>()) - neg;
}

[[nodiscard]] static constexpr std::pair<Vec2i, Vec2i> RegionRange(RegionId id) noexcept
{
    auto sz = GridRegion::kSize.Cast<int32_t>();
    auto begin = id * sz;
    return {begin, begin + sz};
}

struct RegionIdHasher
{
    uint64_t operator()(const RegionId& v) const
    {
        return (uint64_t{std::bit_cast<uint32_t>(v.x())} << 32) | uint64_t{std::bit_cast<uint32_t>(v.y())};
    }
};

struct RegionIdCmp
{
    [[nodiscard]] constexpr bool operator()(const RegionId& a, const RegionId& b) const noexcept
    {
        return a.Tuple() < b.Tuple();
    }
};

struct ParticleData
{
    edt::Vec4u8 color{};
    edt::Vec2i pos{};
};

struct ParticleGrid
{
    bool HasParticleAt(Vec2i p) const
    {
        auto region_id = ToRegionId(p);
        auto it = containers.find(region_id);
        if (it != containers.end())
        {
            auto [begin, end] = RegionRange(region_id);
            auto in_region = (p - begin).Cast<uint32_t>();
            auto& container = it->second;
            return container.bits[in_region.y()].Get(in_region.x());
        }

        return false;
    }

    ParticleId AddParticleAt(ParticleData data)
    {
        auto region_id = ToRegionId(data.pos);
        order.insert(region_id);
        auto& container = containers[region_id];

        auto [begin, end] = RegionRange(region_id);
        auto in_region = (data.pos - begin).Cast<uint32_t>();
        auto id = std::exchange(next_particle_id, ParticleId::FromValue(next_particle_id.GetValue() + 1));
        particles.emplace(id, data);
        container.bits[in_region.y()].Set(in_region.x(), true);
        container.ids[in_region.y()][in_region.x()] = id;

        return id;
    }

    ParticleData RemoveParticleAt(Vec2i p)
    {
        auto region_id = ToRegionId(p);
        auto& container = containers.at(region_id);
        auto [begin, end] = RegionRange(region_id);
        auto in_region = (p - begin).Cast<uint32_t>();
        container.bits[in_region.y()].Set(in_region.x(), false);
        auto id = std::exchange(container.ids[in_region.y()][in_region.x()], ParticleId{});
        return particles.extract(id).mapped();
    }

    std::set<RegionId, RegionIdCmp> order;
    std::unordered_map<RegionId, GridRegion, RegionIdHasher> containers;
    std::unordered_map<ParticleId, ParticleData, TaggedIdHasher<ParticleId>> particles;
    ParticleId next_particle_id = ParticleId::FromValue(0);
};

static_assert(ToRegionId({0, 0}) == Vec2i{0, 0});
static_assert(ToRegionId({-1, 0}) == Vec2i{-1, 0});
static_assert(ToRegionId({-256, 0}) == Vec2i{-1, 0});
static_assert(ToRegionId({-257, 0}) == Vec2i{-2, 0});
static_assert(RegionRange({-1, 0}) == std::pair{Vec2i{-256, 0}, Vec2i{0, 256}});

[[nodiscard]] static constexpr Vec2f FromImVec(ImVec2 v) noexcept
{
    return {v.x, v.y};
}

class Painter2dApp : public Application
{
    static constexpr edt::Vec4u8 red{255, 0, 0, 255};
    static constexpr edt::Vec4u8 green{0, 255, 0, 255};
    static constexpr edt::Vec4u8 blue{0, 0, 255, 255};
    static constexpr edt::Vec4u8 white{255, 255, 255, 255};
    static constexpr float kParticleSize = 0.01f;

    void Initialize() override
    {
        Application::Initialize();

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&Painter2dApp::OnMouseScroll>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Painter 2d");
        painter_ = std::make_unique<Painter2d>();

        this->SetTargetFramerate(90.f);
    }

    void Simulate()
    {
        constexpr Vec2i kRight = Vec2i::AxisX();
        constexpr Vec2i kLeft = -kRight;
        constexpr Vec2i kUp = Vec2i::AxisY();
        constexpr Vec2i kDown = -kUp;

        for (auto& region_id : grid.order)
        {
            auto& cont = grid.containers[region_id];
            for (uint32_t y = 0; y != cont.kSize.y(); ++y)
            {
                for (uint32_t x = 0; x != cont.kSize.x(); ++x)
                {
                    if (cont.bits[y].Get(x))
                    {
                        auto [begin, end] = RegionRange(region_id);
                        const auto particle = begin + Vec2u32{x, y}.Cast<int>();
                        std::optional<Vec2i> dst;
                        if (particle.y() > -100)
                        {
                            // auto left_pt = pi + kLeft;
                            // auto right_pt = pi + kRight;
                            auto down_pt = particle + kDown;
                            // auto up_pt = pi + kUp;
                            auto down_left_pt = particle + kDown + kLeft;
                            auto down_right_pt = particle + kDown + kRight;

                            bool has_down = grid.HasParticleAt(down_pt);
                            bool has_down_left = grid.HasParticleAt(down_left_pt);
                            bool has_down_right = grid.HasParticleAt(down_right_pt);
                            if (!has_down)
                            {
                                dst = down_pt;
                            }
                            else if (!has_down_left)
                            {
                                dst = down_left_pt;
                            }
                            else if (!has_down_right)
                            {
                                dst = down_right_pt;
                            }
                        }

                        if (dst)
                        {
                            auto data = grid.RemoveParticleAt(particle);
                            data.pos = *dst;
                            grid.AddParticleAt(data);
                        }
                    }
                }
            }
        }
    }

    void Tick() override
    {
        Simulate();

        auto viewport = Viewport::FromWindowSize(GetWindow().GetSize());
        render_transforms.Update(camera, viewport, AspectRatioPolicy::ShrinkToFit);

        painter_->BeginDraw();
        painter_->SetViewMatrix(render_transforms.world_to_view.Transposed());

        for (auto& region_id : grid.order)
        {
            auto [begin, end] = RegionRange(region_id);
            auto reg_sz = GridRegion::kSize.Cast<float>() * kParticleSize;
            auto reg_begin = begin.Cast<float>() * kParticleSize;  // FIXED
            auto reg_center = reg_begin + reg_sz * 0.5f;

            painter_->RectLines(
                {.center = reg_center, .size = reg_sz, .color = blue},
                {.inner = 0.01f, .outer = 0.01f});

            auto& cont = grid.containers[region_id];
            for (uint32_t y = 0; y != cont.kSize.y(); ++y)
            {
                for (uint32_t x = 0; x != cont.kSize.x(); ++x)
                {
                    if (cont.bits[y].Get(x))
                    {
                        auto particle = begin + Vec2u32{x, y}.Cast<int>();
                        auto pf = (particle.Cast<float>() * kParticleSize);
                        painter_->FillRect(
                            {.center = pf,
                             .size = Vec2f{kParticleSize, kParticleSize},
                             .color = red,
                             .rotation_degrees = 0});
                    }
                }
            }
        }

        for (auto& [id, data] : grid.particles)
        {
            auto pf = (data.pos.Cast<float>() * kParticleSize);
            painter_->FillRect(
                {.center = pf,
                 .size = Vec2f{kParticleSize, kParticleSize},
                 .color = data.color,
                 .rotation_degrees = 0});
        }

        HandleInput();
        painter_->EndDraw();

        ImGui::Text("Fps: %f", static_cast<double>(GetFramerate()));  // NOLINT
        // ImGui::Text("Fps: %zu", particles.size());                    // NOLINT
        ImGui::Text("Count: %zu", grid.particles.size());  // NOLINT
    }

    void HandleInput()
    {
        if (!ImGui::GetIO().WantCaptureKeyboard)
        {
            int right = 0;
            int up = 0;
            if (ImGui::IsKeyDown(ImGuiKey_W)) up += 1;
            if (ImGui::IsKeyDown(ImGuiKey_S)) up -= 1;
            if (ImGui::IsKeyDown(ImGuiKey_D)) right += 1;
            if (ImGui::IsKeyDown(ImGuiKey_A)) right -= 1;
            if (std::abs(right) + std::abs(up))
            {
                edt::Vec2f delta{};
                delta += static_cast<float>(right) * edt::Vec2f::AxisX();
                delta += static_cast<float>(up) * edt::Vec2f::AxisY();
                camera.eye = camera.eye + delta * move_speed_ * GetLastFrameDurationSeconds() / camera.zoom;
            }
        }

        constexpr Vec2i offset{10, 10};
        auto mouse_position = GetMousePositionInWorldCoordinates();
        auto center = (mouse_position / kParticleSize).Cast<int>();
        auto begin = center - offset, end = center + offset;
        painter_->RectLines(
            {.center = center.Cast<float>() * kParticleSize,
             .size = (offset * 2).Cast<float>() * kParticleSize,
             .color = green,
             .rotation_degrees = 0},
            {.inner = 0.01f});
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
        {
            for (auto p = begin; p.y() != end.y(); ++p.y())
            {
                for (p.x() = begin.x(); p.x() != end.x(); ++p.x())
                {
                    if (!grid.HasParticleAt(p))
                    {
                        grid.AddParticleAt({.color = red, .pos = p});
                    }
                }
            }
        }
    }

    Vec2f GetMousePositionInWorldCoordinates() const
    {
        const auto screen_size = GetWindow().GetSize2f();
        Vec2f p = FromImVec(ImGui::GetMousePos());
        p.y() = screen_size.y() - p.y();
        return edt::Math::TransformPos(render_transforms.screen_to_world, p);
    }

    void OnMouseScroll(const klgl::events::OnMouseScroll& event)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        zoom_power_ += event.value.y();
        camera.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
    }

    std::unique_ptr<Painter2d> painter_;
    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    Camera2d camera{};
    RenderTransforms2d render_transforms{};

    // float line_width_ = 0.001f;
    float move_speed_ = 0.5f;
    float zoom_power_ = 0.f;

    ParticleGrid grid;
    // std::vector<Vec2i> particles;
};

void Main()
{
    Painter2dApp app;
    app.Run();
}
}  // namespace klgl::painter2d_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::painter2d_example::Main);
    return 0;
}
