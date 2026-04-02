#include "Bootstrap/ServerRuntime.h"

#include "DB/DBConnector.h"
#include "Shared/Logging/Logger.h"

#include <memory>

namespace
{
std::unique_ptr<ServerRuntime> GRuntime;
}

void ServerRuntime::Initialize(const ServerConfig& config)
{
    GRuntime.reset(new ServerRuntime(config));
}

ServerRuntime& ServerRuntime::Get()
{
    return *GRuntime;
}

AuthService& ServerRuntime::GetAuthService()
{
    return m_authService;
}

AdminMonitoringService& ServerRuntime::GetAdminMonitoringService()
{
    return m_adminMonitoringService;
}

MatchResultDispatcher& ServerRuntime::GetMatchResultDispatcher()
{
    return m_matchResultDispatcher;
}

ServerRuntime::ServerRuntime(const ServerConfig& config)
    : m_config(config)
    , m_authService(m_userRepository, m_redisCache, m_tokenService)
    , m_adminMonitoringService(m_redisCache)
    , m_matchResultDispatcher(m_matchRepository)
{
    const bool redisConnected = m_redisCache.Connect(config.Redis.Host, config.Redis.Port);
    const bool dbConnected = DBConnector::Get().Connect(config.Mysql.Host,
                                                        config.Mysql.Port,
                                                        config.Mysql.User,
                                                        config.Mysql.Password,
                                                        config.Mysql.Database);
    Logger::Write(redisConnected ? LogLevel::Info : LogLevel::Warning,
                  "bootstrap",
                  redisConnected ? "redis cache initialized" : "redis cache is not available");
    Logger::Write(dbConnected ? LogLevel::Info : LogLevel::Warning,
                  "bootstrap",
                  dbConnected ? "db connector initialized" : "db connector is not available");
}
