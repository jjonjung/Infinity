#pragma once

#include "Infrastructure/Repositories/MatchRepository.h"
#include "Shared/Types/ServiceResult.h"

class MatchResultDispatcher
{
public:
    explicit MatchResultDispatcher(const MatchRepository& matchRepository);

    ServiceResult<VoidValue> PersistDedicatedMatchResult(const PersistMatchResultRequest& request) const;

private:
    const MatchRepository& m_matchRepository;
};
