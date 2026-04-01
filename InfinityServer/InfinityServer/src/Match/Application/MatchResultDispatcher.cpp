#include "Match/Application/MatchResultDispatcher.h"

MatchResultDispatcher::MatchResultDispatcher(const MatchRepository& matchRepository)
    : m_matchRepository(matchRepository)
{
}

ServiceResult<VoidValue> MatchResultDispatcher::PersistDedicatedMatchResult(
    const PersistMatchResultRequest& request) const
{
    return m_matchRepository.PersistMatchResult(request);
}
