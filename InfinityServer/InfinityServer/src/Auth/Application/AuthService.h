#pragma once

#include "Auth/Domain/AuthenticatedUser.h"
#include "Infrastructure/Cache/RedisCache.h"
#include "Infrastructure/Repositories/UserRepository.h"
#include "Shared/Security/TokenService.h"
#include "Shared/Types/ServiceResult.h"

#include <string>

struct LocalLoginRequest
{
    std::string Email;
    std::string Password;
};

struct RegisterRequest
{
    std::string Email;
    std::string Password;
    std::string Nickname;
};

struct SocialLoginRequest
{
    std::string ProviderName;
    std::string ProviderAccessToken;
};

struct TokenBundle
{
    std::string AccessToken;
    std::string RefreshToken;
    std::string GameSessionToken;
};

struct AuthResponse
{
    AuthenticatedUser User;
    TokenBundle Tokens;
};

class AuthService
{
public:
    AuthService(const UserRepository& userRepository,
                RedisCache& redisCache,
                const TokenService& tokenService);

    ServiceResult<AuthResponse> RegisterLocalAccount(const RegisterRequest& request) const;
    ServiceResult<AuthResponse> LoginWithLocalAccount(const LocalLoginRequest& request) const;
    ServiceResult<AuthResponse> LoginWithSocialProvider(const SocialLoginRequest& request) const;
    ServiceResult<AuthenticatedUser> ValidateGameSessionToken(const std::string& gameSessionToken) const;

private:
    AuthResponse BuildAuthResponse(const AuthenticatedUser& user) const;

    const UserRepository& m_userRepository;
    RedisCache& m_redisCache;
    const TokenService& m_tokenService;
};
