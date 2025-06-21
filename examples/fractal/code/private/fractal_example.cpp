#include <fmt/format.h>
#include <imgui.h>

#include <EverydayTools/Math/Math.hpp>
#include <klgl/events/event_listener_interface.hpp>
#include <klgl/events/event_listener_method.hpp>
#include <klgl/events/event_manager.hpp>
#include <klgl/events/mouse_events.hpp>
#include <klgl/template/on_scope_leave.hpp>
#include <klgl/ui/simple_type_widget.hpp>
#include <random>

#include "klgl/application.hpp"
#include "klgl/camera/camera_2d.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/window.hpp"

struct FractalParams
{
    float a = 0.7785f;
    float b = 0.7785f;
    float c = 0.3f;
    float d = 0.5f;
    float time = 0.0f;
    bool use_current_time = true;
    int color_seed = 1234;
    bool interpolate_colors = true;
    std::array<edt::Vec3f, 10> colors;
    std::vector<float> color_positions;
    bool inside_out_space = true;
    bool changed = true;
    int color_mode = 0;

    void RandomizeColors()
    {
        std::mt19937 rnd(static_cast<unsigned>(color_seed));
        std::uniform_real_distribution<float> color_distr(0, 1.0f);

        for (auto& color : colors)
        {
            color = color.Transform([&](float) { return color_distr(rnd); });
        }
    }

    edt::Vec2f MakeJuliaConstant(float current_time)
    {
        if (use_current_time) time = current_time;
        return edt::Vec2f{a * std::cos(time * c), b * std::sin(time * d)};
    }

    template <typename Callback>
    void ComputeColors(size_t n, Callback cb) const
    {
        std::vector<size_t> indices(colors.size());
        for (size_t i = 0; i != colors.size(); ++i) indices[i] = i;

        constexpr std::less pred{};
        auto proj = [&](size_t i) -> const float&
        {
            return color_positions[i];
        };

        std::ranges::sort(indices, pred, proj);

        const size_t last = color_positions.size() - 1;
        for (size_t i = 0; i != n; ++i)
        {
            const float fi = static_cast<float>(i) / static_cast<float>(n - 1);
            const auto li = std::ranges::lower_bound(indices, fi, pred, proj);
            const auto left = std::distance(li, indices.end()) < 2 ? last - 1 : *li;
            const auto right = left + 1;

            if (interpolate_colors)
            {
                float t = (fi - color_positions[left]) / (color_positions[right] - color_positions[left]);
                auto color = edt::Math::Lerp(colors[left], colors[right], t);
                cb(i, color);
            }
            else
            {
                bool use_left = fi - color_positions[left] < color_positions[right] - fi;
                auto color = colors[use_left ? left : right];
                cb(i, color);
            }
        }
    }
};

class FractalApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();
        event_listener_ = klgl::events::EventListenerMethodCallbacks<&FractalApp::OnMouseScroll>::CreatePtr(this);
        GetEventManager().AddEventListener(*event_listener_);

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Fractal");
        SetTargetFramerate(30.f);

        // Create quad mesh
        const auto mesh_data = klgl::ProceduralMeshGenerator::GenerateQuadMesh();
        mesh_ = klgl::MeshOpenGL::MakeFromData(
            std::span{mesh_data.vertices},
            std::span{mesh_data.indices},
            mesh_data.topology);
        mesh_->Bind();

        // Vertex buffer attributes
        klgl::OpenGl::EnableVertexAttribArray(0);
        klgl::OpenGl::VertexAttribPointer(
            0,
            edt::Vec2f::Size(),
            klgl::GlVertexAttribComponentType::Float,
            false,
            sizeof(edt::Vec2f),
            nullptr);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("fractal");

        auto max_iterations_def = *shader_->FindDefine(klgl::Name("MAX_ITERATIONS"));
        max_iterations = static_cast<size_t>(shader_->GetDefineValue<int>(max_iterations_def));
        shader_->Use();

        settings_.RandomizeColors();
        u_color_table.resize(max_iterations + 1);
        settings_.color_positions.resize(settings_.colors.size(), 0.f);

        float delta = 1.f / static_cast<float>(settings_.color_positions.size() - 1);
        for (size_t i = 1; i != settings_.color_positions.size() - 1; ++i)
        {
            settings_.color_positions[i] = static_cast<float>(i) * delta;
        }

        settings_.color_positions.back() = 1.f;
        size_t uniforms_count = u_color_table.size();
        for (size_t i = 0; i != uniforms_count; ++i)
        {
            auto uniform_name = klgl::Name(fmt::format("uColorTable[{}]", i));
            u_color_table[i] = shader_->GetUniform(uniform_name);
        }
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
                camera_.eye = camera_.eye + delta * move_speed_ * GetLastFrameDurationSeconds() / camera_.zoom;
            }
        }
    }

    void Tick() override
    {
        klgl::Application::Tick();
        ApplySettings();

        HandleInput();
        viewport_.MatchWindowSize(GetWindow().GetSize2f());
        render_transforms_.Update(camera_, viewport_);

        shader_->Use();

        shader_->SetUniform(u_screen_to_world_, render_transforms_.screen_to_world.Transposed());
        shader_->SetUniform(u_julia_constant, settings_.MakeJuliaConstant(GetTimeSeconds()));
        if (u_resolution_) shader_->SetUniform(*u_resolution_, GetWindow().GetSize2f());
        if (u_time_) shader_->SetUniform(*u_time_, GetTimeSeconds());

        shader_->SendUniforms();

        mesh_->BindAndDraw();

        DrawGUI();
    }

    void OnMouseScroll(const klgl::events::OnMouseScroll& event)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        zoom_power_ += event.value.y();
        camera_.zoom = std::max(std::pow(1.1f, zoom_power_), 0.1f);
    }

    void ApplySettings()
    {
        if (!std::exchange(settings_.changed, false)) return;

        shader_->SetDefineValue<int>(def_inside_out_space, settings_.inside_out_space ? 1 : 0);
        shader_->SetDefineValue<int>(def_color_mode, settings_.color_mode);
        shader_->Compile();

        u_resolution_ = shader_->FindUniform(klgl::Name("u_resolution"));
        u_time_ = shader_->FindUniform(klgl::Name("u_time"));

        settings_.ComputeColors(
            u_color_table.size(),
            [&](size_t index, const edt::Vec3f& color) { shader_->SetUniform(u_color_table[index], color); });
    }

    void DrawGUI()
    {
        if (ImGui::Begin("Settings"))
        {
            auto& c = settings_.changed;
            if (ImGui::CollapsingHeader("Julia constant"))
            {
                ImGui::SliderFloat("a", &settings_.a, 0.00001f, 1.f);
                ImGui::SliderFloat("b", &settings_.b, 0.00001f, 1.f);
                ImGui::SliderFloat("c", &settings_.c, 0.00001f, 1.f);
                ImGui::SliderFloat("d", &settings_.d, 0.00001f, 1.f);
                ImGui::Checkbox("use current time", &settings_.use_current_time);
            }

            c |= ImGui::Checkbox("Inside out space", &settings_.inside_out_space);
            c |= ImGui::SliderInt("Color Mode", &settings_.color_mode, 0, 2);

            if (ImGui::CollapsingHeader("Colors"))
            {
                for (size_t color_index = 0; color_index != settings_.colors.size(); ++color_index)
                {
                    constexpr int color_edit_flags = ImGuiColorEditFlags_DefaultOptions_ |
                                                     ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
                    auto& color = settings_.colors[color_index];
                    ImGui::PushID(&color);
                    auto pop_on_exit = klgl::OnScopeLeave(&ImGui::PopID);
                    c |= ImGui::ColorEdit3("Color", color.data(), color_edit_flags);

                    if (color_index != 0 && color_index != settings_.colors.size() - 1)
                    {
                        ImGui::SameLine();
                        c |= ImGui::SliderFloat("Pos", &settings_.color_positions[color_index], 0.0f, 1.f);
                    }
                }

                c |= ImGui::Checkbox("Interpolate colors", &settings_.interpolate_colors);

                bool randomize = false;

                if (ImGui::InputInt("Color Seed:", &settings_.color_seed))
                {
                    randomize = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("Randomize"))
                {
                    randomize = true;
                    settings_.color_seed = std::bit_cast<int>(std::random_device()());
                }

                if (randomize)
                {
                    settings_.RandomizeColors();
                    c = true;
                }
            }
        }
        ImGui::End();
    }

    klgl::DefineHandle def_inside_out_space{klgl::Name("INSIDE_OUT_SPACE")};
    klgl::DefineHandle def_color_mode{klgl::Name("COLOR_MODE")};
    std::optional<klgl::UniformHandle> u_time_;
    std::optional<klgl::UniformHandle> u_resolution_;
    klgl::UniformHandle u_screen_to_world_ = klgl::UniformHandle("u_screen_to_world");
    klgl::UniformHandle u_julia_constant = klgl::UniformHandle("u_julia_constant");
    std::vector<klgl::UniformHandle> u_color_table;

    std::unique_ptr<klgl::events::IEventListener> event_listener_;
    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    size_t max_iterations = 100;
    float zoom_power_ = 0.f;
    float move_speed_ = 0.5f;
    klgl::Camera2d camera_;
    klgl::RenderTransforms2d render_transforms_;
    klgl::Viewport viewport_;
    FractalParams settings_;
};

void Main()
{
    FractalApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
