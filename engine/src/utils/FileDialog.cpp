#include "utils/FileDialog.hpp"
#include <cstring>
#include <vector>


// 使用 tinyfiledialogs 实现跨平台文件对话框
#define TINYFD_NOLIB
#include "tinyfiledialogs.h"

namespace
{
// 将字符串过滤器转换为 tinyfiledialogs 需要的格式
void ParseFilter(const std::string &filter, std::vector<const char *> &descriptions,
                 std::vector<const char *> &patterns)
{
    if (filter.empty())
        return;

    // 临时存储分割后的字符串
    std::vector<std::string> tempStorage;

    // 使用 '\0' 分割字符串
    size_t start = 0;
    size_t end = filter.find('\0');
    bool isDescription = true;

    while (end != std::string::npos)
    {
        std::string token = filter.substr(start, end - start);
        if (!token.empty())
        {
            tempStorage.push_back(token);
            if (isDescription)
            {
                descriptions.push_back(tempStorage.back().c_str());
            }
            else
            {
                patterns.push_back(tempStorage.back().c_str());
            }
        }

        start = end + 1;
        end = filter.find('\0', start);
        isDescription = !isDescription;
    }

    // 处理最后一个片段
    if (start < filter.length())
    {
        std::string token = filter.substr(start);
        if (!token.empty())
        {
            tempStorage.push_back(token);
            if (isDescription)
            {
                descriptions.push_back(tempStorage.back().c_str());
            }
            else
            {
                patterns.push_back(tempStorage.back().c_str());
            }
        }
    }

    // 确保描述和模式数量匹配
    if (descriptions.size() > patterns.size())
    {
        descriptions.pop_back();
    }
}
} // namespace

std::string FileDialog::OpenFile(const std::string &title, const std::string &filter)
{
    std::vector<const char *> descriptions;
    std::vector<const char *> patterns;
    ParseFilter(filter, descriptions, patterns);

    const char *result = tinyfd_openFileDialog(title.c_str(),                                       // 对话框标题
                                               nullptr,                                             // 初始路径
                                               static_cast<int>(patterns.size()),                   // 过滤器数量
                                               patterns.data(),                                     // 过滤模式数组
                                               descriptions.size() > 0 ? descriptions[0] : nullptr, // 默认过滤器描述
                                               0                                                    // 单选模式
    );

    return result ? std::string(result) : "";
}

std::string FileDialog::SaveFile(const std::string &title, const std::string &filter)
{
    std::vector<const char *> descriptions;
    std::vector<const char *> patterns;
    ParseFilter(filter, descriptions, patterns);

    const char *result = tinyfd_saveFileDialog(title.c_str(),                                      // 对话框标题
                                               nullptr,                                            // 初始路径
                                               static_cast<int>(patterns.size()),                  // 过滤器数量
                                               patterns.data(),                                    // 过滤模式数组
                                               descriptions.size() > 0 ? descriptions[0] : nullptr // 默认过滤器描述
    );

    return result ? std::string(result) : "";
}