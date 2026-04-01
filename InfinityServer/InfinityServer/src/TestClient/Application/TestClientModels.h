#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ClientLoginTokens
{
    std::string AccessToken;
    std::string RefreshToken;
    std::string GameSessionToken;
};

struct ClientRegisterRequest
{
    std::string Email;
    std::string Password;
    std::string Nickname;
};

struct ClientLoginRequest
{
    std::string Email;
    std::string Password;
};

struct ClientSocialLoginRequest
{
    std::string Provider;
    std::string ProviderToken;
};

struct ClientMatchPlayer
{
    int64_t UserId = 0;
    std::string Team;
    std::string CharacterName;
    int Kills = 0;
    int Deaths = 0;
    int Assists = 0;
    int DamageDealt = 0;
    int DamageTaken = 0;
    int Score = 0;
    std::string Result;
};

struct ClientMatchRequest
{
    std::string MatchId;
    std::string WinnerTeam;
    std::vector<ClientMatchPlayer> Players;
};

struct ClientPlayerStats
{
    int64_t UserId = 0;
    int TotalMatches = 0;
    int TotalWins = 0;
    int TotalKills = 0;
    int TotalDeaths = 0;
    int TotalAssists = 0;
    int TotalDamageDealt = 0;
};

struct ClientOperationResult
{
    bool Success = false;
    int64_t UserId = 0;
    uint8_t Provider = 0;
    std::string Message;
    ClientLoginTokens Tokens;
    ClientPlayerStats Stats;
};

struct TestScenarioExecution
{
    std::string Name;
    bool Success = false;
    std::string Message;
};
