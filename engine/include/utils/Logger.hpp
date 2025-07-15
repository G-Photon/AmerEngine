#pragma once

#include <string>

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR
};

class Logger
{
  public:
    static void Log(LogLevel level, const std::string &message);

    template <typename... Args> static void Info(const char *format, Args... args)
    {
        Log(LogLevel::INFO, FormatString(format, args...));
    }

    template <typename... Args> static void Warning(const char *format, Args... args)
    {
        Log(LogLevel::WARNING, FormatString(format, args...));
    }

    template <typename... Args> static void Error(const char *format, Args... args)
    {
        Log(LogLevel::ERROR, FormatString(format, args...));
    }

  private:
    static std::string FormatString(const char *format, ...);
};