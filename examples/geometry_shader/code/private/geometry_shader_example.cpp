#include <EverydayTools/Math/Math.hpp>
#include <random>

#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/opengl/gl_api.hpp"
#include "klgl/opengl/vertex_attribute_helper.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"
#include "klgl/template/member_offset.hpp"
#include "klgl/window.hpp"

enum class ShapeType : uint8_t
{
    Quad = 0,
    Circle,
    Triangle
};

struct Object
{
    edt::Mat3f transform;
    edt::Vec4u8 color;
    ShapeType type = ShapeType::Quad;
};

class GeometryShaderApp : public klgl::Application
{
    void Initialize() override
    {
        klgl::Application::Initialize();

        klgl::OpenGl::SetClearColor({});
        GetWindow().SetSize(2000, 2000);
        GetWindow().SetTitle("Geometry Shader Quads");

        klgl::OpenGl::EnableBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader_ = std::make_unique<klgl::Shader>("points_to_quads_2d");
        shader_->Use();

        vao_ = klgl::GlObject<klgl::GlVertexArrayId>::CreateFrom(klgl::OpenGl::GenVertexArray());
        klgl::OpenGl::BindVertexArray(vao_);

        vbo_ = klgl::GlObject<klgl::GlBufferId>::CreateFrom(klgl::OpenGl::GenBuffer());
        klgl::OpenGl::BindBuffer(klgl::GlBufferType::Array, vbo_);

        a_transform_ = shader_->GetInfo().VerifyAndGetVertexAttributeLocation<edt::Mat3f>("a_transform");
        {
            using AttribHelper = klgl::VertexBufferHelperStatic<edt::Mat3f>;
            AttribHelper::EnableVertexAttribArray(a_transform_);
            AttribHelper::AttributePointer(a_transform_, sizeof(Object), klgl::MemberOffset<&Object::transform>());
            AttribHelper::AttributeDivisor(a_transform_, 1);
        }

        a_color_ = shader_->GetInfo().VerifyAndGetVertexAttributeLocation<edt::Vec4f>("a_color");
        {
            using AttribHelper = klgl::VertexBufferHelperStatic<edt::Vec4u8, true>;
            AttribHelper::EnableVertexAttribArray(a_color_);
            AttribHelper::AttributePointer(a_color_, sizeof(Object), klgl::MemberOffset<&Object::color>());
            AttribHelper::AttributeDivisor(a_color_, 1);
        }

        a_type_ = shader_->GetInfo().VerifyAndGetVertexAttributeLocation<uint8_t>("a_type");
        {
            using AttribHelper = klgl::VertexBufferHelperStatic<uint8_t, false, false>;
            AttribHelper::EnableVertexAttribArray(a_type_);
            AttribHelper::AttributePointer(a_type_, sizeof(Object), klgl::MemberOffset<&Object::type>());
            AttribHelper::AttributeDivisor(a_type_, 1);
        }

        klgl::OpenGl::BufferData(klgl::GlBufferType::Array, std::span{objects_}, klgl::GlUsage::DynamicDraw);

        objects_.resize(1000);
        std::mt19937 rnd;  // NOLINT
        std::uniform_int_distribution<uint8_t> type_distribution(0, 2);
        for (auto& object : objects_)
        {
            object.type = static_cast<ShapeType>(type_distribution(rnd));
        }
    }

    void Tick() override
    {
        klgl::Application::Tick();

        float color_width_ = 0.15f;
        float spiral_rotation = GetTimeSeconds();
        float p = color_width_ + (1.f - color_width_) * std::abs(std::sin(GetTimeSeconds()));

        size_t n = 1 + static_cast<size_t>(999 * std::abs(std::sin(GetTimeSeconds() / 4)));
        for (const size_t i : std::views::iota(size_t{0}, n))
        {
            const float fi = static_cast<float>(i);
            float k = fi / static_cast<float>(n - 1);
            float rotation_around_origin = edt::Math::DegToRad(15.f - 6 * k) * fi - spiral_rotation;

            auto model = edt::Math::ScaleMatrix(edt::Vec2f{1, 1} * (0.01f + k * 0.02f));
            model = edt::Math::RotationMatrix2d(-GetTimeSeconds() * k * 10.f - rotation_around_origin).MatMul(model);
            model = edt::Math::TranslationMatrix(edt::Vec2f{0.03f, 0} + 0.85f * k).MatMul(model);
            model = edt::Math::RotationMatrix2d(rotation_around_origin).MatMul(model);

            objects_[i].transform = model.Transposed();
            objects_[i].color = edt::Math::GetRainbowColorsA(
                (std::clamp(k, p - color_width_, p + color_width_) + color_width_ - p) * 6);
            klgl::OpenGl::BufferData(klgl::GlBufferType::Array, std::span{objects_}, klgl::GlUsage::DynamicDraw);
        }

        klgl::OpenGl::DrawArraysInstanced(klgl::GlPrimitiveType::Points, 0, 1, n);
    }

    std::shared_ptr<klgl::Shader> shader_;
    klgl::GlObject<klgl::GlVertexArrayId> vao_;

    size_t a_transform_{};
    size_t a_color_{};
    size_t a_type_{};
    klgl::GlObject<klgl::GlBufferId> vbo_;
    std::vector<Object> objects_;
};

void Main()
{
    GeometryShaderApp app;
    app.Run();
}

int main()
{
    klgl::ErrorHandling::InvokeAndCatchAll(Main);
    return 0;
}
