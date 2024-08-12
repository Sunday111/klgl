#pragma once

#include "gl_api.hpp"

#ifdef KLGL_CASE_RET_STR
#error "something wrong - macro name collision!"
#endif

#define KLGL_CASE_RET_STR(str) \
    case str:                  \
        return #str;           \
        break

namespace klgl
{
constexpr std::string_view OpenGl::ToString(GLint value) noexcept
{
    switch (value)
    {
        KLGL_CASE_RET_STR(GL_RGBA);
        KLGL_CASE_RET_STR(GL_RGB);
        KLGL_CASE_RET_STR(GL_RG);
    default:
        return "Unknown value";
        break;
    }
}

constexpr std::string_view OpenGl::ToString(GLenum value) noexcept
{
    switch (value)
    {
        KLGL_CASE_RET_STR(GL_UNSIGNED_BYTE);
        KLGL_CASE_RET_STR(GL_UNSIGNED_INT);
        KLGL_CASE_RET_STR(GL_BYTE);
        KLGL_CASE_RET_STR(GL_INT);
        KLGL_CASE_RET_STR(GL_FLOAT);
    default:
        return "Unknown value";
        break;
    }
}

}  // namespace klgl

#undef KLGL_CASE_RET_STR
