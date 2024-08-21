#include <EverydayTools/Math/Math.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
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
    static MeshVertex FromMeshData(const klgl::MeshData& data, const size_t index);
    edt::Vec2f position{};
    edt::Vec2f texture_coordinates{};
};

class TwoTexturesApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Two textures");

        GenerateMesh();
        PrepareShader();
        GenerateTextures();

        // Vertex buffer attributes
        klgl::OpenGl::EnableVertexAttribArray(0);
        klgl::RegisterAttribute<&MeshVertex::position>(0, false);
        klgl::RegisterAttribute<&MeshVertex::texture_coordinates>(1, false);
    }

    void GenerateMesh()
    {
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
    }

    void PrepareShader()
    {
        // Load shader
        shader_ = std::make_unique<klgl::Shader>("two_textures.shader.json");
        shader_->Use();

        // Obtain uniform handles
        u_color_ = *shader_->FindUniform(klgl::Name("u_color"));
        u_scale_ = *shader_->FindUniform(klgl::Name("u_scale"));
        u_translation_ = *shader_->FindUniform(klgl::Name("u_translation"));
        u_texture_a_ = *shader_->FindUniform(klgl::Name("u_texture_a"));
        u_texture_b_ = *shader_->FindUniform(klgl::Name("u_texture_b"));

        // Set initial uniform parameter for scale uniform. It wont change anymore
        shader_->SetUniform(u_scale_, edt::Vec2f{0.5f, 0.5f});

        // Uniform parameters saved in shader objects and will be sent to driver only after this call
        shader_->SendUniforms();
    }

    void GenerateTextures()
    {
        // Generate triangle mask textur and mirror it
        constexpr auto texture_size = edt::Vec2<size_t>{} + 128;
        auto pixels = klgl::ProceduralTextureGenerator::TriangleMask(texture_size, 2);

        right_triangle_texture_ = klgl::Texture::CreateEmpty(texture_size, klgl::GlTextureInternalFormat::R8);
        right_triangle_texture_->SetPixels<klgl::GlPixelBufferLayout::R>(std::span<const uint8_t>{pixels});

        klgl::ProceduralTextureGenerator::MirrorX(texture_size, pixels);
        left_triangle_texture_ = klgl::Texture::CreateEmpty(texture_size, klgl::GlTextureInternalFormat::R8);
        left_triangle_texture_->SetPixels<klgl::GlPixelBufferLayout::R>(std::span<const uint8_t>{pixels});

        // Generate circle mask texture
        pixels = klgl::ProceduralTextureGenerator::CircleMask(texture_size, 2);
        circle_texture_ = klgl::Texture::CreateEmpty(texture_size, klgl::GlTextureInternalFormat::R8);
        circle_texture_->SetPixels<klgl::GlPixelBufferLayout::R>(std::span<const uint8_t>{pixels});
    }

    void Tick() override
    {
        klgl::Application::Tick();

        // These two calls are unnecessary here but doesnt hurt to keep them
        mesh_->Bind();
        shader_->Use();

        shader_->SetUniform(u_color_, edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);

        shader_->SetUniform(u_texture_a_, *circle_texture_);
        shader_->SetUniform(u_texture_b_, *right_triangle_texture_);
        shader_->SetUniform(u_translation_, edt::Vec2f{0.5f, 0.f});
        shader_->SendUniforms();
        mesh_->Draw();

        shader_->SetUniform(u_texture_a_, *circle_texture_);
        shader_->SetUniform(u_texture_b_, *left_triangle_texture_);
        shader_->SetUniform(u_translation_, edt::Vec2f{-0.5f, 0.f});
        shader_->SendUniforms();
        mesh_->Draw();
    }

    klgl::UniformHandle u_color_;
    klgl::UniformHandle u_scale_;
    klgl::UniformHandle u_translation_;
    klgl::UniformHandle u_texture_a_;
    klgl::UniformHandle u_texture_b_;
    std::shared_ptr<klgl::Shader> shader_;
    std::shared_ptr<klgl::MeshOpenGL> mesh_;
    std::unique_ptr<klgl::Texture> circle_texture_;
    std::unique_ptr<klgl::Texture> right_triangle_texture_;
    std::unique_ptr<klgl::Texture> left_triangle_texture_;
};

void Main()
{
    TwoTexturesApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
