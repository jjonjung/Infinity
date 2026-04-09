#include "Infrastructure/Repositories/MatchRepository.h"

#include "DB/DBConnector.h"

ServiceResult<VoidValue> MatchRepository::PersistMatchResult(const PersistMatchResultRequest& request) const
{
    if (request.MatchId.empty())
    {
        return ServiceResult<VoidValue>::Fail(ServiceErrorCode::InvalidArgument,
                                              "match id is required");
    }

    DbMatch match;
    match.ExternalMatchId = request.MatchId;
    match.WinnerTeam = request.WinnerTeam;
    match.PlayerCount = request.PlayerCount;
    match.Players.reserve(request.Players.size());

    for (const MatchPlayerResult& player : request.Players)
    {
        match.Players.push_back({
            player.UserId,
            player.Team,
            player.CharacterName,
            player.Kills,
            player.Deaths,
            player.Assists,
            player.DamageDealt,
            player.DamageTaken,
            player.Score,
            player.Result
        });
    }

    if (!DBConnector::Get().PersistMatch(match))
    {
        return ServiceResult<VoidValue>::Fail(ServiceErrorCode::ExternalDependencyFailure,
                                              "failed to persist match");
    }

    return ServiceResult<VoidValue>::Ok(VoidValue{});
}

ServiceResult<PlayerAggregateStats> MatchRepository::GetAggregateStats(int64_t userId) const
{
    const auto aggregate = DBConnector::Get().GetAggregateForUser(userId);
    if (!aggregate.has_value())
    {
        return ServiceResult<PlayerAggregateStats>::Fail(ServiceErrorCode::NotFound,
                                                         "aggregate stats not found");
    }

    PlayerAggregateStats stats;
    stats.UserId = aggregate->UserId;
    stats.TotalMatches = aggregate->TotalMatches;
    stats.TotalWins = aggregate->TotalWins;
    stats.TotalKills = aggregate->TotalKills;
    stats.TotalDeaths = aggregate->TotalDeaths;
    stats.TotalAssists = aggregate->TotalAssists;
    stats.TotalDamageDealt = aggregate->TotalDamageDealt;
    return ServiceResult<PlayerAggregateStats>::Ok(stats);
}
