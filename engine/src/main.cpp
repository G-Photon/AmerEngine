#include "core/Application.hpp"

int main()
{
    // 创建应用程序实例并运行
    setlocale(LC_ALL, "zh_CN.UTF-8"); // 设置中文环境
    Application app;
    app.Run();
    return 0;
}