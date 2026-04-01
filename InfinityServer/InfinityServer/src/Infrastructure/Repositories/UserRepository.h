#pragma once

#include "Auth/Domain/AuthenticatedUser.h"
#include "Shared/Types/ServiceResult.h"

#include <string>

struct RegisterLocalUserRequest
{
    std::string Email;
    std::string Password;
    std::string Nickname;
};

class UserRepository
{
public:
    UserRepository();

    ServiceResult<AuthenticatedUser> RegisterLocal(const RegisterLocalUserRequest& request) const;
    ServiceResult<AuthenticatedUser> AuthenticateLocal(const std::string& email,
                                                       const std::string& password) const;
    ServiceResult<AuthenticatedUser> AuthenticateSocial(const std::string& providerName,
                                                        const std::string& providerToken) const;
};
