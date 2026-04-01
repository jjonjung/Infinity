#pragma once

#include <cstdint>
#include <string>

class TokenService
{
public:
    std::string IssueAccessToken(int64_t userId) const;
    std::string IssueRefreshToken(int64_t userId) const;
    std::string IssueGameSessionToken(int64_t userId) const;
};
