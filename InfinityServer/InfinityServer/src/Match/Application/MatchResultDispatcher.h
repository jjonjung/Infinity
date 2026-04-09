#pragma once

#include "Infrastructure/Cache/RedisCache.h"
#include "Infrastructure/Repositories/MatchRepository.h"
#include "Shared/Types/ServiceResult.h"

class MatchResultDispatcher
{
public:
    MatchResultDispatcher(const MatchRepository& matchRepository,
                          RedisCache& redisCache);

    ServiceResult<VoidValue> PersistDedicatedMatchResult(const PersistMatchResultRequest& request) const;

private:
    const MatchRepository& m_matchRepository;
    RedisCache& m_redisCache;
};
