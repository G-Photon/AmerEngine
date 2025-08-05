#include "utils/FileDialog.hpp"
#include <cstring>
#include <string>
#include <vector>


#define TINYFD_NOLIB
#include "tinyfiledialogs.h"

namespace
{
// 转换过滤器格式为 tinyfiledialogs 要求的格式
std::vector<const char *> ConvertFilter(const char *filter, std::string &firstDescription)
{
    std::vector<const char *> result;
    if (!filter || *filter == '\0')
        return result;

    const char *p = filter;
    while (*p != '\0')
    {
        // 添加描述
        result.push_back(p);
        p += strlen(p) + 1;

        // 添加模式（Windows 需要格式如 "*.txt"）
        result.push_back(p);
        p += strlen(p) + 1;
    }

    // 获取第一个描述用于对话框
    if (!result.empty())
    {
        firstDescription = result[0];
    }

    return result;
}
} // namespace

std::string FileDialog::OpenFile(const std::string &title, const char *filter)
{
    std::string firstDescription;
    std::vector<const char *> filterItems = ConvertFilter(filter, firstDescription);

    
    const char **filterPatterns = filterItems.empty() ? nullptr : filterItems.data();
    int numFilterPatterns = static_cast<int>(filterItems.size() / 2);

    const char *result = tinyfd_openFileDialog(title.c_str(), nullptr, numFilterPatterns, filterPatterns,
                                               firstDescription.empty() ? nullptr : firstDescription.c_str(), 0);

    return result ? std::string(result) : "";
}

std::string FileDialog::SaveFile(const std::string &title, const char *filter)
{
    std::string firstDescription;
    std::vector<const char *> filterItems = ConvertFilter(filter, firstDescription);

    
    const char **filterPatterns = filterItems.empty() ? nullptr : filterItems.data();
    int numFilterPatterns = static_cast<int>(filterItems.size() / 2);

    const char *result = tinyfd_saveFileDialog(title.c_str(), nullptr, numFilterPatterns, filterPatterns,
                                               firstDescription.empty() ? nullptr : firstDescription.c_str());

    return result ? std::string(result) : "";
}