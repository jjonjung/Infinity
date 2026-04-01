#include "Auth/Application/AuthService.h"

#include "DB/DBConnector.h"

AuthService::AuthService(const UserRepository& userRepository,
                         RedisCache& redisCache,
                         const TokenService& tokenService)
    : m_userRepository(userRepository)
    , m_redisCache(redisCache)
    , m_tokenService(tokenService)
{
}

ServiceResult<AuthResponse> AuthService::RegisterLocalAccount(const RegisterRequest& request) const
{
    RegisterLocalUserRequest registerRequest;
    registerRequest.Email = request.Email;
    registerRequest.Password = request.Password;
    registerRequest.Nickname = request.Nickname;

    auto userResult = m_userRepository.RegisterLocal(registerRequest);
    if (!userResult.Success)
    {
        return ServiceResult<AuthResponse>::Fail(userResult.Error.Code, userResult.Error.Message);
    }

    return ServiceResult<AuthResponse>::Ok(BuildAuthResponse(userResult.Value));
}

ServiceResult<AuthResponse> AuthService::LoginWithLocalAccount(const LocalLoginRequest& request) const
{
    if (request.Email.empty() || request.Password.empty())
    {
        return ServiceResult<AuthResponse>::Fail(ServiceErrorCode::InvalidArgument,
                                                 "email and password are required");
    }

    auto userResult = m_userRepository.AuthenticateLocal(request.Email, request.Password);
    if (!userResult.Success)
    {
        return ServiceResult<AuthResponse>::Fail(userResult.Error.Code, userResult.Error.Message);
    }

    return ServiceResult<AuthResponse>::Ok(BuildAuthResponse(userResult.Value));
}

ServiceResult<AuthResponse> AuthService::LoginWithSocialProvider(const SocialLoginRequest& request) const
{
    if (request.ProviderName.empty() || request.ProviderAccessToken.empty())
    {
        return ServiceResult<AuthResponse>::Fail(ServiceErrorCode::InvalidArgument,
                                                 "provider and access token are required");
    }

    auto userResult = m_userRepository.AuthenticateSocial(request.ProviderName,
                                                          request.ProviderAccessToken);
    if (!userResult.Success)
    {
        return ServiceResult<AuthResponse>::Fail(userResult.Error.Code, userResult.Error.Message);
    }

    return ServiceResult<AuthResponse>::Ok(BuildAuthResponse(userResult.Value));
}

ServiceResult<AuthenticatedUser> AuthService::ValidateGameSessionToken(const std::string& gameSessionToken) const
{
    if (gameSessionToken.empty())
    {
        return ServiceResult<AuthenticatedUser>::Fail(ServiceErrorCode::InvalidArgument,
                                                      "game session token is required");
    }

    const auto userId = m_redisCache.FindUserIdByGameSessionToken(gameSessionToken);
    if (!userId.has_value())
    {
        return ServiceResult<AuthenticatedUser>::Fail(ServiceErrorCode::Unauthorized,
                                                      "invalid or expired game session token");
    }

    AuthenticatedUser user;
    user.UserId = *userId;
    user.DisplayName = "ValidatedUser";
    user.Provider = LoginProvider::Local;
    return ServiceResult<AuthenticatedUser>::Ok(user);
}

AuthResponse AuthService::BuildAuthResponse(const AuthenticatedUser& user) const
{
    AuthResponse response;
    response.User = user;
    response.Tokens.AccessToken = m_tokenService.IssueAccessToken(user.UserId);
    response.Tokens.RefreshToken = m_tokenService.IssueRefreshToken(user.UserId);
    response.Tokens.GameSessionToken = m_tokenService.IssueGameSessionToken(user.UserId);
    m_redisCache.StoreGameSessionToken(response.Tokens.GameSessionToken, user.UserId);
    DBConnector::Get().SaveSession({ user.UserId, response.Tokens.RefreshToken, response.Tokens.GameSessionToken });
    return response;
}
