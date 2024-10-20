#include "klgl/camera/camera_2d.hpp"

#include "klgl/opengl/gl_api.hpp"

namespace klgl
{
void Viewport::UseInOpenGL()
{
    auto p = position.Cast<GLint>();
    auto s = size.Cast<GLint>();
    glViewport(p.x(), p.y(), s.x(), s.y());
}
}  // namespace klgl
