#include "klgl/filesystem/filesystem.hpp"

#include <fmt/format.h>
#include <fmt/std.h>

#include <fstream>

namespace klgl
{

void Filesystem::ReadFile(const std::filesystem::path& path, std::string& buffer)
{
    buffer.clear();
    AppendFileContentToBuffer(path, buffer);
}

void Filesystem::AppendFileContentToBuffer(const std::filesystem::path& path, std::string& buffer)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    [[unlikely]] if (!file.is_open())
    {
        throw std::runtime_error(fmt::format("failed to open file {}", path));
    }
    const std::streamsize read_size = file.tellg();
    file.seekg(0, std::ios::beg);

    const size_t prev_size = buffer.size();
    buffer.resize(prev_size + static_cast<size_t>(read_size));

    [[unlikely]] if (!file.read(buffer.data() + prev_size, read_size))  // NOLINT
    {
        throw std::runtime_error(fmt::format("failed to read {} bytes from file {}", read_size, path));
    }
}

}  // namespace klgl
