#pragma once

#include <string>

class PasswordHasher
{
public:
    static std::string HashForDevelopment(const std::string& plainText);
    static bool VerifyForDevelopment(const std::string& plainText, const std::string& hashed);
};
