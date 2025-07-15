#include "utils/Logger.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cstdarg>


void Logger::Log(LogLevel level, const std::string &message)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::cout << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";

    switch (level)
    {
    case LogLevel::INFO:
        std::cout << "[INFO] ";
        break;
    case LogLevel::WARNING:
        std::cout << "[WARNING] ";
        break;
    case LogLevel::ERROR:
        std::cout << "[ERROR] ";
        break;
    }

    std::cout << message << std::endl;
}

std::string Logger::FormatString(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    size_t size = std::vsnprintf(nullptr, 0, format, args) + 1; // +1 for null terminator
    std::string str(size, '\0');
    std::vsnprintf(&str[0], size, format, args);
    va_end(args);
    // Remove the null terminator for std::string
    if (!str.empty() && str.back() == '\0') {
        str.pop_back();
    }
    return str;
}