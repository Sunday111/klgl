#pragma once

#include <filesystem>
#include <string>

namespace klgl
{

class Filesystem
{
public:
    static void ReadFile(const std::filesystem::path& path, std::string& buffer);
    static void AppendFileContentToBuffer(const std::filesystem::path& path, std::string& buffer);
};

}  // namespace klgl
