#pragma once

#include "Auth/Domain/LoginProvider.h"

#include <cstdint>
#include <string>

struct AuthenticatedUser
{
    int64_t UserId = 0;
    std::string DisplayName;
    LoginProvider Provider = LoginProvider::Local;
};
