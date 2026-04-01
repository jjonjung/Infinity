#pragma once

#include "Shared/Types/ServiceResult.h"

#include <cstdint>
#include <string>
#include <vector>

struct MatchPlayerResult
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

struct PersistMatchResultRequest
{
    std::string MatchId;
    std::string WinnerTeam;
    int PlayerCount = 0;
    std::vector<MatchPlayerResult> Players;
};

struct PlayerAggregateStats
{
    int64_t UserId = 0;
    int TotalMatches = 0;
    int TotalWins = 0;
    int TotalKills = 0;
    int TotalDeaths = 0;
    int TotalAssists = 0;
    int TotalDamageDealt = 0;
};

class MatchRepository
{
public:
    ServiceResult<VoidValue> PersistMatchResult(const PersistMatchResultRequest& request) const;
    ServiceResult<PlayerAggregateStats> GetAggregateStats(int64_t userId) const;
};
