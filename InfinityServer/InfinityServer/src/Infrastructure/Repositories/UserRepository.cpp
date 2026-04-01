#include "Infrastructure/Repositories/UserRepository.h"

#include "DB/DBConnector.h"
#include "Shared/Security/PasswordHasher.h"

namespace
{
LoginProvider ToProvider(const std::string& providerName)
{
    if (providerName == "google")
    {
        return LoginProvider::Google;
    }

    if (providerName == "steam")
    {
        return LoginProvider::Steam;
    }

    return LoginProvider::Local;
}

AuthenticatedUser ToUser(const DbUser& user, LoginProvider provider)
{
    AuthenticatedUser authenticatedUser;
    authenticatedUser.UserId = user.UserId;
    authenticatedUser.DisplayName = user.Nickname;
    authenticatedUser.Provider = provider;
    return authenticatedUser;
}
}

UserRepository::UserRepository()
{
}

ServiceResult<AuthenticatedUser> UserRepository::RegisterLocal(const RegisterLocalUserRequest& request) const
{
    if (request.Email.empty() || request.Password.empty() || request.Nickname.empty())
    {
        return ServiceResult<AuthenticatedUser>::Fail(ServiceErrorCode::InvalidArgument,
                                                      "email, password, and nickname are required");
    }

    const std::string hashedPassword = PasswordHasher::HashForDevelopment(request.Password);
    const auto createdUser = DBConnector::Get().CreateLocalUser(request.Email, request.Nickname, hashedPassword);
    if (!createdUser.has_value())
    {
        return ServiceResult<AuthenticatedUser>::Fail(ServiceErrorCode::Conflict,
                                                      "email or nickname already exists");
    }

    return ServiceResult<AuthenticatedUser>::Ok(ToUser(*createdUser, LoginProvider::Local));
}

ServiceResult<AuthenticatedUser> UserRepository::AuthenticateLocal(const std::string& email,
                                                                   const std::string& password) const
{
    const auto user = DBConnector::Get().AuthenticateLocalUser(email, password);
    if (!user.has_value())
    {
        return ServiceResult<AuthenticatedUser>::Fail(ServiceErrorCode::Unauthorized,
                                                      "invalid email or password");
    }

    return ServiceResult<AuthenticatedUser>::Ok(ToUser(*user, LoginProvider::Local));
}

ServiceResult<AuthenticatedUser> UserRepository::AuthenticateSocial(const std::string& providerName,
                                                                    const std::string& providerToken) const
{
    const auto user = DBConnector::Get().AuthenticateSocialUser(providerName, providerToken);
    if (!user.has_value())
    {
        return ServiceResult<AuthenticatedUser>::Fail(ServiceErrorCode::Unauthorized,
                                                      "invalid social token");
    }

    return ServiceResult<AuthenticatedUser>::Ok(ToUser(*user, ToProvider(providerName)));
}
