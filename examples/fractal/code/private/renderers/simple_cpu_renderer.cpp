#include "simple_cpu_renderer.hpp"

#include <klgl/template/register_attribute.hpp>
#include <klgl/texture/texture.hpp>

#include "fractal_settings.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"

SimpleCpuRenderer::SimpleCpuRenderer(size_t max_iterations_) : max_iterations(max_iterations_)
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

    mesh_ = klgl::MeshOpenGL::MakeFromData(std::span{vertices}, std::span{mesh_data.indices}, mesh_data.topology);
    mesh_->Bind();

    shader_ = std::make_unique<klgl::Shader>("fractal_example/textured_quad");
    shader_->Use();

    // Verify vertex attributes types match your expectations and get their location by name
    const auto& program_info = shader_->GetInfo();
    a_vertex_ = program_info.VerifyAndGetVertexAttributeLocation<edt::Vec2f>("a_vertex");
    a_tex_coord_ = program_info.VerifyAndGetVertexAttributeLocation<edt::Vec2f>("a_tex_coord");

    // Declare vertex buffer layout
    klgl::RegisterAttribute<&MeshVertex::position>(a_vertex_);
    klgl::RegisterAttribute<&MeshVertex::texture_coordinates>(a_tex_coord_);
}

SimpleCpuRenderer::~SimpleCpuRenderer() noexcept = default;

void SimpleCpuRenderer::Render(const FractalSettings& settings)
{
    klgl::OpenGl::SetViewport(settings.viewport);
    shader_->Use();

    auto s = settings.viewport.size.Cast<size_t>();
    auto [w, h] = s.Tuple();
    auto num_pixels = w * h;
    image_buffer_.resize(num_pixels);

    auto julia_constant = settings.MakeJuliaConstant();
    for (size_t y = 0; y != h; ++y)
    {
        for (size_t x = 0; x != w; ++x)
        {
            auto& pixel = image_buffer_[y * w + x];
            edt::Vec2<size_t> frag_coord_u{x, y};
            auto frag_coord_f = frag_coord_u.Cast<float>();

            auto c = julia_constant;
            auto world = edt::Math::TransformPos(render_transforms_.screen_to_world, frag_coord_f);

            auto z = world;

            size_t i = 0;
            while (i != max_iterations)
            {
                auto p = edt::Math::ComplexPower(z, settings.fractal_power).value_or(edt::Vec2f{}) + c;
                if (p.SquaredLength() > 4) break;
                z = p;
                ++i;
            }

            pixel = pallette[i];
        }
    }
    render_transforms_.Update(settings.camera, settings.viewport);
    texture_->SetPixels<klgl::GlPixelBufferLayout::RGB>(std::span<const edt::Vec3f>{image_buffer_});
    shader_->SetUniform(u_texture_, *texture_);

    shader_->SendUniforms();
    texture_->Bind();
    mesh_->BindAndDraw();
}

void SimpleCpuRenderer::ApplySettings(const FractalSettings& settings)
{
    if (auto s = settings.viewport.size.Cast<size_t>(); !texture_ || texture_->GetSize() != s)
    {
        texture_ =
            klgl::Texture::CreateEmpty(settings.viewport.size.Cast<size_t>(), klgl::GlTextureInternalFormat::RGB8);
    }

    pallette.resize(max_iterations + 1);
    settings.ComputeColors(pallette.size(), [&](size_t index, const edt::Vec3f& color) { pallette[index] = color; });
}
