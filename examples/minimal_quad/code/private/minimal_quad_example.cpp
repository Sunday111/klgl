#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
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
            edt::Vec2f::Size(),
            klgl::GlVertexAttribComponentType::Float,
            false,
            sizeof(edt::Vec2f),
            nullptr);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("just_color_2d");
        shader_->Use();
    }

    void Tick() override
    {
        klgl::Application::Tick();

        shader_->Use();
        auto m = edt::Math::ScaleMatrix(edt::Vec2f{} + 0.5f);
        m = edt::Math::RotationMatrix2d(GetTimeSeconds()).MatMul(m);
        m = edt::Math::TranslationMatrix(edt::Vec2f{0.5, 0}).MatMul(m);
        m = m.Transposed();
        shader_->SetUniform(u_transform_, m);
        shader_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
        shader_->SendUniforms();

        mesh_->BindAndDraw();
    }

    klgl::UniformHandle u_color_ = klgl::UniformHandle("u_color");
    klgl::UniformHandle u_transform_ = klgl::UniformHandle("u_transform");
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
