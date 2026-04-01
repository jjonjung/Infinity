#include "Shared/Logging/Logger.h"

#include <iostream>

namespace
{
const char* ToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}
}

void Logger::Write(LogLevel level, const std::string& category, const std::string& message)
{
    std::cout << "[" << ToString(level) << "]"
              << "[" << category << "] "
              << message << '\n';
}
