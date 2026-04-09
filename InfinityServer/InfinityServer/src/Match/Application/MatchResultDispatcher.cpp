#include "Match/Application/MatchResultDispatcher.h"

#include <sstream>

MatchResultDispatcher::MatchResultDispatcher(const MatchRepository& matchRepository,
                                             RedisCache& redisCache)
    : m_matchRepository(matchRepository)
    , m_redisCache(redisCache)
{
}

ServiceResult<VoidValue> MatchResultDispatcher::PersistDedicatedMatchResult(
    const PersistMatchResultRequest& request) const
{
    auto result = m_matchRepository.PersistMatchResult(request);
    if (!result.Success)
    {
        return result;
    }

    std::ostringstream summary;
    summary << "winner=" << request.WinnerTeam << ",players=" << request.PlayerCount;
    m_redisCache.CacheMatchSnapshot(request.MatchId, summary.str());

    for (const MatchPlayerResult& player : request.Players)
    {
        m_redisCache.CacheLeaderboardEntry("current", player.UserId, player.Score);
    }

    return result;
}
