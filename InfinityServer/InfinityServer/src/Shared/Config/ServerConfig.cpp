#include "Shared/Config/ServerConfig.h"

#include <cstdlib>

namespace
{
std::string ReadEnvOrDefault(const char* name, const std::string& fallback)
{
    char* value = nullptr;
    std::size_t length = 0;
    const errno_t result = _dupenv_s(&value, &length, name);
    if (result != 0 || value == nullptr || value[0] == '\0')
    {
        free(value);
        return fallback;
    }

    std::string resolvedValue(value);
    free(value);
    return resolvedValue;
}

uint16_t ReadEnvPortOrDefault(const char* name, uint16_t fallback)
{
    char* value = nullptr;
    std::size_t length = 0;
    const errno_t result = _dupenv_s(&value, &length, name);
    if (result != 0 || value == nullptr || value[0] == '\0')
    {
        free(value);
        return fallback;
    }

    const auto port = static_cast<uint16_t>(std::strtoul(value, nullptr, 10));
    free(value);
    return port;
}
}

ServerConfig ServerConfigLoader::LoadDefaults()
{
    ServerConfig config;
    config.Mysql.Host = ReadEnvOrDefault("INFINITY_MYSQL_HOST", config.Mysql.Host);
    config.Mysql.Port = ReadEnvPortOrDefault("INFINITY_MYSQL_PORT", config.Mysql.Port);
    config.Mysql.Database = ReadEnvOrDefault("INFINITY_MYSQL_DATABASE", config.Mysql.Database);
    config.Mysql.User = ReadEnvOrDefault("INFINITY_MYSQL_USER", config.Mysql.User);
    config.Mysql.Password = ReadEnvOrDefault("INFINITY_MYSQL_PASSWORD", config.Mysql.Password);
    return config;
}
