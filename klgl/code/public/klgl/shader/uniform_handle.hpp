#pragma once

#include "klgl/name_cache/name.hpp"

namespace klgl
{

class UniformHandle
{
public:
    UniformHandle() = default;
    explicit UniformHandle(std::string_view name) : name(Name(name)) {}
    UniformHandle(uint32_t in_index, Name in_name) : index(in_index), name(in_name) {}

    uint32_t index = 0;
    Name name{};
};

}  // namespace klgl
