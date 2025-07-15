#pragma once

#include <string>

class FileSystem
{
  public:
    static std::string GetPath(const std::string &path);
    static std::string GetDirectory(const std::string &path);
    static std::string GetFilename(const std::string &path);
    static bool FileExists(const std::string &path);
};