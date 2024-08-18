#pragma once

#include <cpptrace/cpptrace.hpp>

#include "klgl/opengl/enums.hpp"

namespace klgl
{
class OpenGlError : public cpptrace::runtime_error
{
public:
    OpenGlError(const GlError error, std::string message) : cpptrace::runtime_error(std::move(message)), error_(error)
    {
    }

    GlError GetError() const { return error_; };

private:
    GlError error_;
};
}  // namespace klgl
