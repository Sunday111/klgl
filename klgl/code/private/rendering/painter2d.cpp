#include "klgl/rendering/painter2d.hpp"

#include "EverydayTools/Math/IntRange.hpp"
#include "EverydayTools/Math/Math.hpp"
#include "klgl/application.hpp"
#include "klgl/error_handling.hpp"
#include "klgl/mesh/mesh_data.hpp"
#include "klgl/mesh/procedural_mesh_generator.hpp"
#include "klgl/opengl/vertex_attribute_helper.hpp"
#include "klgl/reflection/matrix_reflect.hpp"  // IWYU pragma: keep
#include "klgl/shader/shader.hpp"

namespace klgl
{

class Painter2d::Impl
{
public:
    static constexpr size_t kBatchSize = 3;
    static constexpr size_t kVertexAttrib = 0;
    static constexpr size_t kTypeAttrib = 1;
    static constexpr size_t kColorAttrib = 2;
    static constexpr size_t kTransformAttrib = 4;

    template <typename ValueType, bool to_float = true, bool normalize = false>
    struct Batch
    {
        using AttribHelper = TightlyPackedAttributeBufferStatic<ValueType, normalize, to_float>;

        Batch() : vbo(GlObject<GlBufferId>::CreateFrom(OpenGl::GenBuffer()))
        {
            OpenGl::BindBuffer(GlBufferType::Array, vbo);
            OpenGl::BufferData(GlBufferType::Array, std::span{values}.size_bytes(), GlUsage::DynamicDraw);
        }

        void Send(const GLuint location, const edt::IntRange<size_t> elements_to_update)
        {
            OpenGl::BindBuffer(GlBufferType::Array, vbo);
            OpenGl::BufferSubData(
                GlBufferType::Array,
                elements_to_update.begin,
                std::span{values}.subspan(elements_to_update.begin));
            AttribHelper::EnableVertexAttribArray(location);
            AttribHelper::AttributePointer(location);
            AttribHelper::AttributeDivisor(location, 1);
        }

        GlObject<GlBufferId> vbo{};
        std::array<ValueType, kBatchSize> values{};
    };

    void BeginDraw()
    {
        ErrorHandling::Ensure(
            !drawing,
            "Trying to start drawing but previous drawing session was not paired with EndDraw call");
        drawing = true;
        num_primitives = 0;
    }

    void EndDraw()
    {
        ErrorHandling::Ensure(drawing, "Attempt to stop drawing twice or without previous BeginDraw call");
        drawing = false;

        shader_->Use();
        OpenGl::EnableBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader_->SendUniforms();
        mesh_->Bind();

        for (size_t batch_index = 0; batch_index != type_batches_.size(); ++batch_index)
        {
            if (num_primitives <= batch_index * kBatchSize) break;

            // number of circles initialized for the current batch
            const size_t num_locally_used = std::min(num_primitives - batch_index * kBatchSize, kBatchSize);
            const edt::IntRange<size_t> update_range{.begin = 0, .end = num_locally_used};

            type_batches_[batch_index].Send(kTypeAttrib, update_range);
            color_batches_[batch_index].Send(kColorAttrib, update_range);
            transform_batches_[batch_index].Send(kTransformAttrib, update_range);

            mesh_->DrawInstanced(num_locally_used);
        }
    }

    void EnsureBeganDrawing() { ErrorHandling::Ensure(drawing, "Did not start drawing session!"); }

    void AddPrimitive(uint8_t shape, const Vec4u8& color, const Mat3f& transform)
    {
        EnsureBeganDrawing();

        if (num_primitives == type_batches_.size() * kBatchSize)
        {
            type_batches_.emplace_back();
            color_batches_.emplace_back();
            transform_batches_.emplace_back();
        }

        const size_t index_in_batch = num_primitives % kBatchSize;
        const size_t batch_index = num_primitives / kBatchSize;

        num_primitives += 1;

        auto& type_batch = type_batches_[batch_index];
        type_batch.values[index_in_batch] = shape;

        auto& color_batch = color_batches_[batch_index];
        color_batch.values[index_in_batch] = color;

        auto& transform_batch = transform_batches_[batch_index];
        transform_batch.values[index_in_batch] = transform.Transposed();
    }

    std::vector<Batch<uint8_t, false>> type_batches_;
    std::vector<Batch<Vec4u8, true, true>> color_batches_;
    std::vector<Batch<Mat3f>> transform_batches_;

    Application* app_ = nullptr;
    std::unique_ptr<Shader> shader_;
    std::shared_ptr<MeshOpenGL> mesh_;

    bool drawing = false;
    size_t num_primitives = 0;
};

struct GlVertexAttributeInfo
{
    std::string name;
    size_t index{};
    size_t location{};
    size_t size{};
    GlVertexAttributeType type{};
};

[[nodiscard]] static std::vector<GlVertexAttributeInfo> GetVertexAttributesInfo(GlProgramId program)
{
    const size_t num_attributes = OpenGl::GetProgramActiveAttributesCount(program);

    std::string attribute_name_buffer;
    attribute_name_buffer.resize(OpenGl::GetProgramActiveAttributeMaxNameLength(program));

    std::vector<GlVertexAttributeInfo> attributes;
    attributes.resize(num_attributes);

    for (const size_t attribute_index : std::views::iota(size_t{0}, num_attributes))
    {
        auto& attribute = attributes[attribute_index];

        size_t attribute_name_length = 0;
        OpenGl::GetActiveAttribute(
            program,
            attribute_index,
            attribute_name_buffer.size(),
            attribute_name_length,
            attribute.size,
            attribute.type,
            attribute_name_buffer.data());

        attribute.index = attribute_index;
        attribute.name = std::string_view{attribute_name_buffer}.substr(0, static_cast<size_t>(attribute_name_length));
        attribute.location = OpenGl::GetAttributeLocation(program, attribute.name);
    }

    return attributes;
}

Painter2d::Painter2d(Application& app) : self(std::make_unique<Impl>())
{
    self->app_ = &app;
    self->shader_ = std::make_unique<Shader>("klgl/painter2d.shader.json");

    for (const auto& attribute : GetVertexAttributesInfo(self->shader_->GetProgramId()))
    {
        fmt::println(
            "Attribute #{}: {}. Location: {}. Size: {}. Type: {}",
            attribute.index,
            attribute.name,
            attribute.location,
            attribute.size,
            attribute.type);
    }

    // Create quad mesh
    const auto mesh_data = ProceduralMeshGenerator::GenerateQuadMesh();

    self->mesh_ =
        MeshOpenGL::MakeFromData(std::span{mesh_data.vertices}, std::span{mesh_data.indices}, mesh_data.topology);
    self->mesh_->Bind();

    // Vertex buffer attributes
    OpenGl::EnableVertexAttribArray(Impl::kVertexAttrib);
    TightlyPackedAttributeBufferStatic<edt::Vec2f, false, true>::AttributePointer(Impl::kVertexAttrib);
}

Painter2d::~Painter2d() = default;

void Painter2d::BeginDraw()
{
    self->BeginDraw();
}

void Painter2d::EndDraw()
{
    self->EndDraw();
}

void Painter2d::DrawRect(const Rect2d& rect)
{
    auto m = edt::Math::ScaleMatrix(rect.size / 2);
    if (rect.rotation_degrees != 0.f)
    {
        const float radians = edt::Math::DegToRad(rect.rotation_degrees);
        m = edt::Math::RotationMatrix2d(radians).MatMul(m);
    }

    m = edt::Math::TranslationMatrix(rect.center).MatMul(m);
    self->AddPrimitive(0, rect.color, m);
}

void Painter2d::DrawCircle(const Circle2d& circle)
{
    auto m = edt::Math::ScaleMatrix(circle.size / 2.f);
    if (circle.rotation_degrees != 0.f)
    {
        const float radians = edt::Math::DegToRad(circle.rotation_degrees);
        m = edt::Math::RotationMatrix2d(radians).MatMul(m);
    }

    m = edt::Math::TranslationMatrix(circle.center).MatMul(m);

    self->AddPrimitive(1, circle.color, m);
}

void Painter2d::DrawTriangle(const Triangle2d& triangle)
{
    // The transformation below is an inlined version of the following algorithm:
    // 1. Translate by 1 so that bottom left corner in screen space (point A) becomes 0, 0
    // 2. Transform the quad space so that x axis becomes AB and y axis becomes AC
    // 3. Move point a to bottom left corner of screen space
    // Mat3f basis;
    // basis.SetColumn(0, Vec3f{i, 0});
    // basis.SetColumn(1, Vec3f{j, 0});
    // basis.SetColumn(2, Vec3f{0, 0, 1});
    // m = edt::Math::TranslationMatrix(triangle.a).MatMul(basis.MatMul(edt::Math::TranslationMatrix(Vec2f{} + 1)));

    const Vec2f i = (triangle.b - triangle.a) / 2;
    const Vec2f j = (triangle.c - triangle.a) / 2;
    const Vec2f t = (triangle.b + triangle.c) / 2;

    Mat3f m;
    m.SetColumn(0, Vec3f{i, 0});
    m.SetColumn(1, Vec3f{j, 0});
    m.SetColumn(2, Vec3f(t, 1));

    self->AddPrimitive(2, triangle.color, m);
}

}  // namespace klgl
