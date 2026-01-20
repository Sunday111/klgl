#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <klgl/events/event_listener_interface.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <klgl/mesh/mesh_data.hpp>
#include <klgl/opengl/vertex_attribute_helper.hpp>
#include <klgl/shader/shader.hpp>
#include <klgl/template/register_attribute.hpp>
#include <klgl/texture/texture.hpp>
#include <mutex>
#include <random>
#include <thread>

#include "fractal_settings.hpp"
#include "klgl/application.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/rendering/curve_renderer_2d.hpp"
#include "klgl/window.hpp"

namespace klgl::curve_example
{

using namespace edt::lazy_matrix_aliases;  // NOLINT

class Framebuffer
{
public:
    void Bind(GlFramebufferBindTarget target = GlFramebufferBindTarget::DrawAndRead)
    {
        OpenGl::BindFramebuffer(target, fbo);
    }

    void CreateWithResolution(const edt::Vec2<size_t>& resolution)
    {
        if (fbo.IsValid())
        {
            OpenGl::DeleteFramebuffer(fbo);
            fbo = {};

            depth_stencil = nullptr;

            OpenGl::DeleteRenderbuffer(rbo_depth_stencil);
            rbo_depth_stencil = {};
        }

        rbo_depth_stencil = OpenGl::GenRenderbuffer();
        OpenGl::BindRenderbuffer(rbo_depth_stencil);
        OpenGl::RenderbufferStorage(GlTextureInternalFormat::DEPTH24_STENCIL8, resolution);

        color = Texture::CreateEmpty(resolution, GlTextureInternalFormat::RGB32F);
        color->Bind();
        OpenGl::SetTextureMinFilter(GlTargetTextureType::Texture2d, GlTextureFilter::Nearest);
        OpenGl::SetTextureMagFilter(GlTargetTextureType::Texture2d, GlTextureFilter::Nearest);
        for (const auto axis : ass::EnumSet<GlTextureWrapAxis>::Full())
        {
            OpenGl::SetTextureWrap(GlTargetTextureType::Texture2d, axis, GlTextureWrapMode::ClampToBorder);
        }

        fbo = OpenGl::GenFramebuffer();
        Bind();

        // Color
        OpenGl::FramebufferTexture2D(
            GlFramebufferBindTarget::DrawAndRead,
            GlFramebufferAttachment::Color0,
            GlTargetTextureType::Texture2d,
            color->GetTexture());

        // Depth stencil
        OpenGl::FramebufferRenderbuffer(
            GlFramebufferBindTarget::DrawAndRead,
            GlFramebufferAttachment::DepthStencil,
            rbo_depth_stencil);

        ErrorHandling::Ensure(
            glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            "Incomplete frambuffer!");
        OpenGl::BindFramebuffer(GlFramebufferBindTarget::DrawAndRead, {});
    }

    GlRenderbufferId rbo_depth_stencil;
    GlFramebufferId fbo{};
    std::unique_ptr<Texture> color{};
    std::unique_ptr<Texture> depth_stencil{};
};

class CurveFractalApp : public Application
{
    static constexpr edt::Vec2<size_t> kFramebufferResolution{3840, 2160};
    std::tuple<int, int> GetOpenGLVersion() const override { return {4, 3}; }
    static constexpr size_t kMaxCurves = 10'000;

    void Initialize() override
    {
        Application::Initialize();

        SetAutoClear(false);

        event_listener_ = klgl::events::EventListenerMethodCallbacks<&CurveFractalApp::OnMouseScroll>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        OpenGl::SetClearColor({});
        GetWindow().SetSize(kFramebufferResolution.x(), kFramebufferResolution.y());
        GetWindow().SetTitle("Curve Fractal");

        OpenGl::EnableBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        constexpr Vec2f eye{0, 0};
        constexpr float sample_extent = 3.f;
        constexpr edt::FloatRange2Df world_range =
            edt::FloatRange2Df::FromMinMax(eye - sample_extent, eye + sample_extent);

        auto thread_tile = world_range.Extent() / 2.f;
        for (size_t x = 0; x != 4; ++x)
        {
            for (size_t y = 0; y != 4; ++y)
            {
                auto tile_min = Vec2<size_t>{x, y}.Cast<float>() * thread_tile + world_range.Min();
                auto thread_range = edt::FloatRange2Df::FromMinMax(tile_min, tile_min + thread_tile);
                producer_thread_.emplace_back(&CurveFractalApp::ProducerThread, this, thread_range);
            }
        }
        textured_quad_shader_ = std::make_unique<Shader>("post_processing_example/textured_quad");
        framebuffer_.CreateWithResolution({3840, 2160});
        framebuffer_.color->Bind();
        klgl::OpenGl::SetTextureMagFilter(GlTargetTextureType::Texture2d, GlTextureFilter::Linear);
        klgl::OpenGl::SetTextureMinFilter(GlTargetTextureType::Texture2d, GlTextureFilter::Linear);

        CreateQuadMesh();
    }

    void ProducerThread(const edt::FloatRange2Df world_range)
    {
        constexpr size_t max_iterations = 2000;

        FractalSettings settings{10};
        settings.camera = camera;
        settings.viewport = Viewport::FromWindowSize(GetWindow().GetSize());
        settings.color_seed = std::bit_cast<int>(std::random_device()());
        settings.RandomizeColors();
        settings.DistributePositionsUniformly();

        std::mt19937_64 rnd{static_cast<unsigned>(settings.color_seed)};
        std::uniform_real_distribution<float> x_distr(world_range.Min().x(), world_range.Max().x());
        std::uniform_real_distribution<float> y_distr(world_range.Min().y(), world_range.Max().y());

        std::vector<CurveRenderer2d::ControlPoint> points;
        std::vector<edt::Vec3f> pallette;
        pallette.resize(max_iterations + 1);
        settings.ComputeColors(
            pallette.size(),
            [&](size_t index, const edt::Vec3f& color) { pallette[index] = color; });

        while (true)
        {
            Vec2f world{x_distr(rnd), y_distr(rnd)};

            auto z = world;

            points.clear();
            points.push_back({
                .position = z,
            });

            size_t i = 0;
            while (i != max_iterations)
            {
                edt::Vec2f p = edt::Math::ComplexPower(z, settings.fractal_power) + settings.fractal_constant;
                points.push_back({
                    .position = edt::Vec2f{z.y(), -z.x()},
                });

                if (p.SquaredLength() > world_range.Extent().SquaredLength()) break;
                z = p;
                ++i;
            }

            if (size_t num_points = points.size(); num_points > 2)
            {
                for (size_t pi = 0; pi != points.size(); ++pi)
                {
                    auto& point = points[pi];
                    point.color = Vec4f(pallette[pi], 1);
                    point.color.w() = (static_cast<float>(pi) / static_cast<float>(num_points)) * 0.2f;
                }

                while (true)
                {
                    std::scoped_lock lock{mutex_};
                    if (num_produced_ == produced_curves_points_.size())
                    {
                        std::this_thread::yield();
                    }
                    else
                    {
                        produced_curves_points_[num_produced_++] = std::move(points);
                        break;
                    }
                }
            }
        }
    }

    void CreateQuadMesh()
    {
        struct MeshVertex
        {
            edt::Vec2f position{};
            edt::Vec2f texture_coordinates{};
        };

        // Create quad mesh
        const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateQuadMesh();

        std::vector<MeshVertex> vertices;
        vertices.reserve(mesh_data.vertices.size());
        for (size_t i = 0; i != mesh_data.vertices.size(); ++i)
        {
            vertices.emplace_back(MeshVertex{
                .position = mesh_data.vertices[i],
                .texture_coordinates = mesh_data.texture_coordinates[i],
            });
        }

        quad_ = klgl::MeshOpenGL::MakeFromData(std::span{vertices}, std::span{mesh_data.indices}, mesh_data.topology);
        quad_->Bind();

        // Declare vertex buffer layout
        klgl::RegisterAttribute<&MeshVertex::position>(0);
        klgl::RegisterAttribute<&MeshVertex::texture_coordinates>(1);
    }

    void Tick() override
    {
        framebuffer_.Bind();
        klgl::OpenGl::SetViewport(klgl::Viewport::FromWindowSize(kFramebufferResolution.Cast<uint32_t>()));

        glDisable(GL_DEPTH_TEST);
        if (first_clear_)
        {
            OpenGl::Clear(GL_COLOR_BUFFER_BIT);
            first_clear_ = false;
        }

        {
            std::scoped_lock lock{mutex_};
            constexpr size_t kMaxCurvesPerFrame = 100;
            const size_t num_curves = std::min(kMaxCurvesPerFrame, num_produced_);
            for (auto& points : std::span{produced_curves_points_}.first(num_produced_).last(num_curves))
            {
                auto& curve = curves_.emplace_back(std::make_unique<klgl::CurveRenderer2d>());
                curve->thickness_ = 1.f;
                curve->SetPoints(points);
                points.clear();
            }
            num_produced_ -= num_curves;
        }

        auto viewport = Viewport::FromWindowSize(GetWindow().GetSize());
        render_transforms.Update(camera, viewport, AspectRatioPolicy::ShrinkToFit);
        // Relaxed memory order as we do not care much if something will be rendered the next frame
        for (auto& curve : curves_)
        {
            curve->Draw(viewport.size.Cast<float>(), render_transforms.world_to_view);
        }

        curves_.clear();

        auto& texture = *framebuffer_.color;
        OpenGl::BindFramebuffer(GlFramebufferBindTarget::DrawAndRead, {});
        klgl::OpenGl::SetViewport(klgl::Viewport::FromWindowSize(GetWindow().GetSize()));
        OpenGl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        textured_quad_shader_->Use();
        textured_quad_shader_->SetUniform(u_textured_quad_shader_texture_, texture);
        textured_quad_shader_->SendUniforms();
        texture.Bind();
        quad_->BindAndDraw();

        HandleInput();
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
    }

    void OnMouseScroll(const klgl::events::OnMouseScroll& event)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        zoom_power_ += event.value.y();
        camera.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
    }

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    Camera2d camera{};
    RenderTransforms2d render_transforms{};

    // float line_width_ = 0.001f;
    float move_speed_ = 0.5f;
    float zoom_power_ = 0.f;

    bool first_clear_ = true;

    std::vector<std::jthread> producer_thread_;

    size_t num_produced_ = 0;
    std::array<std::vector<CurveRenderer2d::ControlPoint>, kMaxCurves> produced_curves_points_;
    std::mutex mutex_;

    std::vector<std::unique_ptr<CurveRenderer2d>> curves_;

    std::shared_ptr<Shader> textured_quad_shader_;
    UniformHandle u_textured_quad_shader_texture_ = UniformHandle("u_texture");
    std::shared_ptr<MeshOpenGL> quad_;

    Framebuffer framebuffer_;
};

void Main()
{
    CurveFractalApp app;
    app.Run();
}
}  // namespace klgl::curve_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::curve_example::Main);
    return 0;
}
