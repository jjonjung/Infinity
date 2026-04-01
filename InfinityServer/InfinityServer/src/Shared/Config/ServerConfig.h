#pragma once

#include <cstdint>
#include <string>

struct AuthServerConfig
{
    uint16_t PublicPort = 7000;
    uint16_t AdminPort = 7100;
    std::string JwtIssuer = "Infinity.Auth";
    std::string AccessTokenTtl = "15m";
    std::string RefreshTokenTtl = "30d";
};

struct GameServerConfig
{
    uint16_t DedicatedPort = 9000;
    std::string Region = "ap-northeast-2";
};

struct RedisConfig
{
    std::string Host = "127.0.0.1";
    uint16_t Port = 6379;
    int SessionTtlSeconds = 900;
    int LeaderboardTtlSeconds = 30;
    int MatchCacheTtlSeconds = 300;
};

struct MysqlConfig
{
    std::string Host = "127.0.0.1";
    uint16_t Port = 3306;
    std::string Database = "infinity";
    std::string User = "app";
    std::string Password = "dev-password";
};

struct ServerConfig
{
    AuthServerConfig Auth;
    GameServerConfig Game;
    RedisConfig Redis;
    MysqlConfig Mysql;
};

class ServerConfigLoader
{
public:
    static ServerConfig LoadDefaults();
};
