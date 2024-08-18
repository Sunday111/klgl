#include "klgl/error_handling.hpp"

#include "klgl/opengl/gl_api.hpp"
#include "klgl/opengl/open_gl_error.hpp"

namespace klgl
{
void ErrorHandling::CheckOpenGlError(const std::string_view context)
{
    [[unlikely]] if (const auto error = OpenGl::GetError(); error != GlError::NoError)
    {
        throw OpenGlError(error, fmt::format("OpenGL error: {}. Context: {}", error, context));
    }
}
}  // namespace klgl
