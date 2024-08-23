#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/window.hpp"

class QuadApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Quad");

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
            2,
            klgl::GlVertexAttribComponentType::Float,
            false,
            sizeof(edt::Vec2f),
            nullptr);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("just_color.shader.json");
        shader_->Use();

        // Set initial uniform parameters of the shader
        shader_->SetUniform(u_color_, edt::Vec4f{1.f, 0.f, 0.f, 1.f});
        shader_->SetUniform(u_scale_, edt::Vec2f{0.5f, 0.5f});
        shader_->SetUniform(u_translation_, edt::Vec2f{0.0f, 0.0f});

        // Uniform parameters saved in shader objects and will be sent to driver only after this call
        shader_->SendUniforms();
    }

    void Tick() override
    {
        klgl::Application::Tick();

        shader_->Use();
        shader_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
        shader_->SendUniform(u_color_);

        mesh_->BindAndDraw();
    }

    klgl::UniformHandle u_color_ = klgl::UniformHandle("u_color");
    klgl::UniformHandle u_scale_ = klgl::UniformHandle("u_scale");
    klgl::UniformHandle u_translation_ = klgl::UniformHandle("u_translation");
    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
};

void Main()
{
    QuadApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
