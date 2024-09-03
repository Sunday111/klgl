#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/template/register_attribute.hpp"
#include "klgl/texture/procedural_texture_generator.hpp"
#include "klgl/texture/texture.hpp"
#include "klgl/window.hpp"

struct MeshVertex
{
    edt::Vec2f position{};
    edt::Vec2f texture_coordinates{};
};

class TexturedQuadApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Textured Quad");

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

        mesh_ = klgl::MeshOpenGL::MakeFromData(std::span{vertices}, std::span{mesh_data.indices}, mesh_data.topology);
        mesh_->Bind();

        // Vertex buffer attributes
        klgl::OpenGl::EnableVertexAttribArray(0);
        klgl::RegisterAttribute<&MeshVertex::position>(0, false);
        klgl::RegisterAttribute<&MeshVertex::texture_coordinates>(1, false);

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("textured_quad_2d.shader.json");
        shader_->Use();

        // Obtain uniform handles
        u_color_ = *shader_->FindUniform(klgl::Name("u_color"));
        u_scale_ = *shader_->FindUniform(klgl::Name("u_scale"));
        u_translation_ = *shader_->FindUniform(klgl::Name("u_translation"));
        u_texture_ = *shader_->FindUniform(klgl::Name("u_texture"));

        // Generate circle mask texture
        {
            constexpr auto size = edt::Vec2<size_t>{} + 128;
            texture_ = klgl::Texture::CreateEmpty(size, klgl::GlTextureInternalFormat::R8);
            const auto pixels = klgl::ProceduralTextureGenerator::CircleMask(size, 2);
            texture_->SetPixels<klgl::GlPixelBufferLayout::R>(std::span{pixels});
            klgl::OpenGl::SetTextureMinFilter(klgl::GlTargetTextureType::Texture2d, klgl::GlTextureFilter::Nearest);
            klgl::OpenGl::SetTextureMagFilter(klgl::GlTargetTextureType::Texture2d, klgl::GlTextureFilter::Linear);
        }

        // Set initial uniform parameters of the shader
        shader_->SetUniform(u_color_, edt::Vec4f{1.f, 0.f, 0.f, 1.f});
        shader_->SetUniform(u_scale_, edt::Vec2f{0.5f, 0.5f});
        shader_->SetUniform(u_translation_, edt::Vec2f{0.0f, 0.0f});
        shader_->SetUniform(u_texture_, *texture_);

        // Uniform parameters saved in shader objects and will be sent to driver only after this call
        shader_->SendUniforms();
    }

    void Tick() override
    {
        klgl::Application::Tick();

        shader_->Use();
        shader_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
        shader_->SendUniform(u_color_);

        texture_->Bind();

        mesh_->BindAndDraw();
    }

    klgl::UniformHandle u_color_;
    klgl::UniformHandle u_scale_;
    klgl::UniformHandle u_translation_;
    klgl::UniformHandle u_texture_;
    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    std::unique_ptr<klgl::Texture> texture_;
};

void Main()
{
    TexturedQuadApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
