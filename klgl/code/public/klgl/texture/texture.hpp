#pragma once

#include <memory>
#include <optional>
#include <span>

#include "klgl/opengl/gl_api.hpp"

namespace klgl
{

class Texture
{
public:
    static std::unique_ptr<Texture> CreateEmpty(size_t width, size_t height, GLint internal_format = GL_RGB);

    ~Texture();

    void Bind() const;
    void SetPixels(std::span<const Vec3<uint8_t>> pixel_data);
    void SetPixels(std::span<const Vec4<uint8_t>> pixel_data);
    Vec2<size_t> GetSize() const { return {width_, height_}; }
    size_t GetWidth() const { return width_; }
    size_t GetHeight() const { return height_; }
    std::optional<GLuint> GetTexture() const { return texture_; }

private:
    struct SetPixelsParameters
    {
        GLenum pixel_data_format = GL_RGB;
        GLenum pixel_data_type = GL_UNSIGNED_BYTE;
        std::span<const uint8_t> pixel_data;
    };
    void SetPixelsImpl(const SetPixelsParameters& p);

private:
    std::optional<GLuint> texture_;
    size_t width_ = 0;
    size_t height_ = 0;
    GLenum type_ = GL_TEXTURE_2D;
};

}  // namespace klgl
