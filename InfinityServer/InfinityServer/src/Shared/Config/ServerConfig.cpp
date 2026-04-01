#include "Shared/Config/ServerConfig.h"

#include <cstdlib>

namespace
{
std::string ReadEnvOrDefault(const char* name, const std::string& fallback)
{
    const char* value = std::getenv(name);
    return (value != nullptr && value[0] != '\0') ? value : fallback;
}

uint16_t ReadEnvPortOrDefault(const char* name, uint16_t fallback)
{
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0')
    {
        return fallback;
    }

    return static_cast<uint16_t>(std::strtoul(value, nullptr, 10));
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
