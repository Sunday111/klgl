#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/opengl/program_info.hpp"
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

        // Load shader
        shader_ = std::make_unique<klgl::Shader>("textured_quad_2d");
        shader_->Use();

        // Verify vertex attributes types match your expectations and get their location by name
        const auto& program_info = shader_->GetInfo();
        a_vertex_ = program_info.VerifyAndGetVertexAttributeLocation<edt::Vec2f>("a_vertex");
        a_tex_coord_ = program_info.VerifyAndGetVertexAttributeLocation<edt::Vec2f>("a_tex_coord");

        // Declare vertex buffer layout
        klgl::RegisterAttribute<&MeshVertex::position>(a_vertex_);
        klgl::RegisterAttribute<&MeshVertex::texture_coordinates>(a_tex_coord_);

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
        shader_->SendUniforms();
        texture_->Bind();
        mesh_->BindAndDraw();
    }

    size_t a_vertex_{};
    size_t a_tex_coord_{};
    klgl::UniformHandle u_color_{"u_color"};
    klgl::UniformHandle u_scale_{"u_scale"};
    klgl::UniformHandle u_translation_{"u_translation"};
    klgl::UniformHandle u_texture_{"u_texture"};
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
