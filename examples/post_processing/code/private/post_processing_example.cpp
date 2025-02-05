#include <EverydayTools/Math/Math.hpp>
#include <klgl/shader/sampler_uniform.hpp>
#include <klgl/template/register_attribute.hpp>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/texture/texture.hpp"
#include "klgl/window.hpp"

namespace klgl::post_processing_example
{

struct MeshVertex
{
    edt::Vec2f position{};
    edt::Vec2f texture_coordinates{};
};

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

class PostProcessingExample : public Application
{
public:
    void Initialize() override
    {
        Application::Initialize();

        OpenGl::SetClearColor({});
        GetWindow().SetSize(1000, 1000);
        GetWindow().SetTitle("Post-processing effect");

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

        // Declare vertex buffer layout
        klgl::RegisterAttribute<&MeshVertex::position>(0);
        klgl::RegisterAttribute<&MeshVertex::texture_coordinates>(1);

        // Load shader
        color_shader_ = std::make_unique<Shader>("post_processing_example/just_color_2d");
        textured_quad_shader_ = std::make_unique<Shader>("post_processing_example/textured_quad");
        blur_shader_ = std::make_unique<Shader>("post_processing_example/blur");
    }

    void UpdateFramebuffer()
    {
        if (const auto resolution = GetWindow().GetSize().Cast<size_t>(); resolution != fbo_resolution_)
        {
            fbo_resolution_ = resolution;
            for (auto& f : framebuffers_)
            {
                f.CreateWithResolution(resolution);
            }
        }
    }

    void Tick() override
    {
        Application::Tick();

        UpdateFramebuffer();

        glDisable(GL_DEPTH_TEST);

        auto render_scene = [&]
        {
            color_shader_->Use();
            auto m = edt::Math::ScaleMatrix(edt::Vec2f{} + 0.5f);
            m = edt::Math::RotationMatrix2d(GetTimeSeconds()).MatMul(m);
            m = edt::Math::TranslationMatrix(edt::Vec2f{0.5, 0}).MatMul(m);
            m = m.Transposed();
            color_shader_->SetUniform(u_color_shader_transform_, m);
            color_shader_->SetUniform(
                u_color_shader_color_,
                edt::Math::GetRainbowColorsA(GetTimeSeconds()).Cast<float>() / 255.f);
            color_shader_->SendUniforms();
            mesh_->BindAndDraw();
        };

        // Render scene to texture
        {
            framebuffers_[0].Bind();
            OpenGl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            render_scene();
        }

        // Pass 1: Do blur passes
        for (size_t i = 0; i != 10; ++i)
        {
            uint32_t is_horizontal = i % 2 != 0;

            framebuffers_[(i + 1) % 2].Bind();  // Store result in fbo[1]
            OpenGl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            auto& texture = *framebuffers_[i % 2].color;

            blur_shader_->Use();
            blur_shader_->SetUniform(u_blur_shader_horizontal_, is_horizontal);
            blur_shader_->SetUniform(u_blur_shader_texture_, texture);
            blur_shader_->SendUniforms();

            texture.Bind();
            mesh_->BindAndDraw();
        }

        // Render to screen
        {
            auto& texture = *framebuffers_[0].color;
            OpenGl::BindFramebuffer(GlFramebufferBindTarget::DrawAndRead, {});
            OpenGl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            textured_quad_shader_->Use();
            textured_quad_shader_->SetUniform(u_textured_quad_shader_texture_, texture);
            textured_quad_shader_->SendUniforms();
            texture.Bind();
            mesh_->BindAndDraw();

            // render_scene(); -> this line converts this example to bloom
        }
    }

    std::shared_ptr<Shader> color_shader_;
    UniformHandle u_color_shader_color_ = UniformHandle("u_color");
    UniformHandle u_color_shader_transform_ = UniformHandle("u_transform");

    std::shared_ptr<Shader> textured_quad_shader_;
    UniformHandle u_textured_quad_shader_texture_ = UniformHandle("u_texture");

    std::shared_ptr<Shader> blur_shader_;
    UniformHandle u_blur_shader_texture_ = UniformHandle("u_texture");
    UniformHandle u_blur_shader_horizontal_ = UniformHandle("u_horizontal");

    std::shared_ptr<MeshOpenGL> mesh_;

    edt::Vec2<size_t> fbo_resolution_{};
    std::array<Framebuffer, 2> framebuffers_{};
};

void Main()
{
    PostProcessingExample app;
    app.Run();
}

}  // namespace klgl::post_processing_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::post_processing_example::Main);
    return 0;
}
