#pragma once

#include "Admin/Application/AdminMonitoringService.h"
#include "Auth/Application/AuthService.h"
#include "Infrastructure/Cache/RedisCache.h"
#include "Infrastructure/Repositories/MatchRepository.h"
#include "Infrastructure/Repositories/UserRepository.h"
#include "Match/Application/MatchResultDispatcher.h"
#include "Shared/Config/ServerConfig.h"
#include "Shared/Security/TokenService.h"

class ServerRuntime
{
public:
    static void Initialize(const ServerConfig& config);
    static ServerRuntime& Get();

    AuthService& GetAuthService();
    AdminMonitoringService& GetAdminMonitoringService();
    MatchResultDispatcher& GetMatchResultDispatcher();

private:
    explicit ServerRuntime(const ServerConfig& config);

    ServerConfig m_config;
    RedisCache m_redisCache;
    UserRepository m_userRepository;
    MatchRepository m_matchRepository;
    TokenService m_tokenService;
    AuthService m_authService;
    AdminMonitoringService m_adminMonitoringService;
    MatchResultDispatcher m_matchResultDispatcher;
};
