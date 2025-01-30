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

namespace klgl::render_to_texture_example
{

struct MeshVertex
{
    edt::Vec2f position{};
    edt::Vec2f texture_coordinates{};
};

class RenderToTextureExampleApp : public Application
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
        color_shader_ = std::make_unique<Shader>("render_to_texture_example/just_color_2d");
        textured_quad_shader_ = std::make_unique<Shader>("render_to_texture_example/textured_quad");

        CreateFramebuffer();
    }

    void CreateFramebuffer()
    {
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        const auto resolution = GetWindow().GetSize().Cast<size_t>();

        fbo_color_ = Texture::CreateEmpty(resolution, GlTextureInternalFormat::RGB32F);
        OpenGl::SetTextureMinFilter(GlTargetTextureType::Texture2d, GlTextureFilter::Nearest);
        OpenGl::SetTextureMagFilter(GlTargetTextureType::Texture2d, GlTextureFilter::Nearest);
        fbo_color_->Bind();
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            fbo_color_->GetTexture().GetValue(),
            0);

        unsigned int rbo{};
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            resolution.Cast<GLsizei>().x(),
            resolution.Cast<GLsizei>().y());  // use a single renderbuffer object for both a depth AND stencil buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        ErrorHandling::Ensure(
            glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            "Incomplete frambuffer!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~RenderToTextureExampleApp() override { glDeleteFramebuffers(1, &fbo_); }

    void Tick() override
    {
        Application::Tick();
        glDisable(GL_DEPTH_TEST);

        // Render to texture
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
            OpenGl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
        }

        // Render to screen
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            OpenGl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            textured_quad_shader_->Use();
            textured_quad_shader_->SetUniform(u_textured_quad_shader_texture_, *fbo_color_);
            float r = std::abs(std::sin(GetTimeSeconds()));
            textured_quad_shader_->SetUniform(u_textured_quad_shader_color_, edt::Vec4f{r, 1, 1, 1});
            textured_quad_shader_->SendUniforms();
            fbo_color_->Bind();
            mesh_->BindAndDraw();
        }
    }

    std::shared_ptr<Shader> color_shader_;
    UniformHandle u_color_shader_color_ = UniformHandle("u_color");
    UniformHandle u_color_shader_transform_ = UniformHandle("u_transform");

    std::shared_ptr<Shader> textured_quad_shader_;
    UniformHandle u_textured_quad_shader_texture_ = UniformHandle("u_texture");
    UniformHandle u_textured_quad_shader_color_ = UniformHandle("u_color");

    std::shared_ptr<MeshOpenGL> mesh_;

    GLuint fbo_{};
    std::unique_ptr<Texture> fbo_color_{};
    std::unique_ptr<Texture> fbo_depth_stencil_{};
};

void Main()
{
    RenderToTextureExampleApp app;
    app.Run();
}

}  // namespace klgl::render_to_texture_example

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(klgl::render_to_texture_example::Main);
    return 0;
}
