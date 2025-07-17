#pragma once
#include <string>


class FileDialog
{
  public:
    // 打开文件对话框
    // title: 对话框标题
    // filter: 文件过滤器，格式为 "描述1\0过滤模式1\0描述2\0过滤模式2\0" (例如: "Image Files\0*.jpg;*.png\0All
    // Files\0*.*\0")
    static std::string OpenFile(const std::string &title, const std::string &filter);

    // 保存文件对话框
    static std::string SaveFile(const std::string &title, const std::string &filter);
};