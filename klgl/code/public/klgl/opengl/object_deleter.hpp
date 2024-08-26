#pragma once

#include "gl_api.hpp"

namespace klgl::detail
{
template <typename Id, auto fn>
struct GlObjectDeleterImpl
{
    [[nodiscard]] std::optional<OpenGlError> operator()(Id id) noexcept { return fn(id); }
};
}  // namespace klgl::detail

namespace klgl
{

template <typename Id>
struct GlObjectDeleter;

template <>
struct GlObjectDeleter<GlTextureId> : detail::GlObjectDeleterImpl<GlTextureId, OpenGl::DeleteTextureCE>
{
};

template <>
struct GlObjectDeleter<GlBufferId> : detail::GlObjectDeleterImpl<GlBufferId, OpenGl::DeleteBufferCE>
{
};

template <>
struct GlObjectDeleter<GlShaderId> : detail::GlObjectDeleterImpl<GlShaderId, OpenGl::DeleteShaderCE>
{
};

template <>
struct GlObjectDeleter<GlProgramId> : detail::GlObjectDeleterImpl<GlProgramId, OpenGl::DeleteProgramCE>
{
};

}  // namespace klgl
