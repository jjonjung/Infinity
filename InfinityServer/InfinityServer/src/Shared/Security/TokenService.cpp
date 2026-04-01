#include "Shared/Security/TokenService.h"

#include <atomic>

namespace
{
std::string IssueToken(const char* prefix, int64_t userId)
{
    static std::atomic<uint64_t> counter = 1;
    return std::string(prefix) + "-" + std::to_string(userId) + "-" + std::to_string(counter++);
}
}

std::string TokenService::IssueAccessToken(int64_t userId) const
{
    return IssueToken("atk", userId);
}

std::string TokenService::IssueRefreshToken(int64_t userId) const
{
    return IssueToken("rtk", userId);
}

std::string TokenService::IssueGameSessionToken(int64_t userId) const
{
    return IssueToken("gsk", userId);
}
