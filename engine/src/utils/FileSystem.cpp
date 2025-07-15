#include "utils/FileSystem.hpp"
#include <filesystem>

namespace fs = std::filesystem;

std::string FileSystem::GetPath(const std::string &path)
{
    // 这里可以添加更多的路径解析逻辑
    if (fs::path(path).is_absolute())
    {
        return path; // 如果是绝对路径，直接返回
    }
    // 如果是相对路径，转换为绝对路径
    return fs::absolute(path).string();
}

std::string FileSystem::GetDirectory(const std::string &path)
{
    return fs::path(path).parent_path().string();
}

std::string FileSystem::GetFilename(const std::string &path)
{
    return fs::path(path).filename().string();
}

bool FileSystem::FileExists(const std::string &path)
{
    return fs::exists(path);
}