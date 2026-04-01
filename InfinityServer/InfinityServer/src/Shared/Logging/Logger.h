#pragma once

#include <string>

enum class LogLevel
{
    Info,
    Warning,
    Error
};

class Logger
{
public:
    static void Write(LogLevel level, const std::string& category, const std::string& message);
};
