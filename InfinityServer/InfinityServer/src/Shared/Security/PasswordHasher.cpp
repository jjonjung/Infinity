#include "Shared/Security/PasswordHasher.h"

#include <functional>

std::string PasswordHasher::HashForDevelopment(const std::string& plainText)
{
    const size_t hashValue = std::hash<std::string>{}(plainText);
    return "devhash:" + std::to_string(hashValue);
}

bool PasswordHasher::VerifyForDevelopment(const std::string& plainText, const std::string& hashed)
{
    return HashForDevelopment(plainText) == hashed;
}
