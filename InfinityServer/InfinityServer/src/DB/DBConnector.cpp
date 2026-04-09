#include "DB/DBConnector.h"

#include "Shared/Security/PasswordHasher.h"

#include <algorithm>
#include <iostream>

namespace sql
{
class Connection
{
};
}

namespace
{
DbUser MakeUser(int64_t userId,
                const std::string& email,
                const std::string& nickname)
{
    DbUser user;
    user.UserId = userId;
    user.Email = email;
    user.Nickname = nickname;
    user.Status = "ACTIVE";
    return user;
}

DbIdentity MakeIdentity(int64_t userId,
                        const std::string& provider,
                        const std::string& providerUserId,
                        const std::string& passwordHash = {})
{
    DbIdentity identity;
    identity.UserId = userId;
    identity.Provider = provider;
    identity.ProviderUserId = providerUserId;
    identity.PasswordHash = passwordHash;
    return identity;
}
}

DBConnector& DBConnector::Get()
{
    static DBConnector instance;
    return instance;
}

DBConnector::DBConnector()
{
}

bool DBConnector::Connect(const std::string& host, int port,
                          const std::string& user,
                          const std::string& password,
                          const std::string& database)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_host = host;
    m_port = port;
    m_user = user;
    m_password = password;
    m_database = database;
    m_connected = !host.empty() && port > 0 && !user.empty() && !database.empty();
    EnsureSchema();
    SeedDevelopmentData();
    std::cout << "[DB] initialized in-memory persistence for mysql schema '" << m_database << "'\n";
    return m_connected;
}

void DBConnector::Disconnect()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connected = false;
}

std::optional<DbUser> DBConnector::CreateLocalUser(const std::string& email,
                                                   const std::string& nickname,
                                                   const std::string& passwordHash)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (EmailExists(email) || NicknameExists(nickname))
    {
        return std::nullopt;
    }

    const int64_t userId = m_nextUserId++;
    StoredUser storedUser;
    storedUser.User = MakeUser(userId, email, nickname);
    storedUser.Identities.push_back(MakeIdentity(userId, "local", email, passwordHash));

    m_userIdByEmail[email] = userId;
    m_userIdByIdentity[{ "local", email }] = userId;
    m_usersById.emplace(userId, storedUser);
    return storedUser.User;
}

std::optional<DbUser> DBConnector::AuthenticateLocalUser(const std::string& email,
                                                         const std::string& plainPassword) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return FindUserByIdentity("local", email, true, plainPassword);
}

std::optional<DbUser> DBConnector::AuthenticateSocialUser(const std::string& provider,
                                                          const std::string& providerUserId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return FindUserByIdentity(provider, providerUserId, false, {});
}

bool DBConnector::SaveSession(const DbSession& session)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto now = std::chrono::system_clock::now();
    auto existing = std::find_if(m_sessions.begin(),
                                 m_sessions.end(),
                                 [&](const StoredSession& stored)
                                 {
                                     return stored.Session.UserId == session.UserId
                                         && stored.Session.RefreshTokenHash == session.RefreshTokenHash;
                                 });
    if (existing != m_sessions.end())
    {
        existing->Session = session;
        existing->CreatedAt = now;
        return true;
    }

    m_sessions.push_back({ session, now });
    return true;
}

bool DBConnector::PersistMatch(const DbMatch& match)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto existing = std::find_if(m_matches.begin(),
                                 m_matches.end(),
                                 [&](const DbMatch& stored)
                                 {
                                     return stored.ExternalMatchId == match.ExternalMatchId;
                                 });

    DbMatch persisted = match;
    if (existing != m_matches.end())
    {
        persisted.MatchId = existing->MatchId;
        *existing = persisted;
        return true;
    }

    persisted.MatchId = m_nextMatchId++;
    m_matches.push_back(persisted);
    return true;
}

std::optional<DbPlayerAggregate> DBConnector::GetAggregateForUser(int64_t userId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_usersById.find(userId) == m_usersById.end())
    {
        return std::nullopt;
    }

    return BuildAggregateForUser(userId);
}

std::unique_ptr<sql::Connection> DBConnector::OpenConnection() const
{
    return {};
}

void DBConnector::EnsureSchema() const
{
}

void DBConnector::SeedDevelopmentData() const
{
    auto* self = const_cast<DBConnector*>(this);
    if (!self->m_usersById.empty())
    {
        return;
    }

    const int64_t googleUserId = self->m_nextUserId++;
    StoredUser googleUser;
    googleUser.User = MakeUser(googleUserId, "google.qa@infinity.local", "GoogleTester");
    googleUser.Identities.push_back(MakeIdentity(googleUserId, "google", "google-dev-token"));
    self->m_usersById.emplace(googleUserId, googleUser);
    self->m_userIdByEmail[googleUser.User.Email] = googleUserId;
    self->m_userIdByIdentity[{ "google", "google-dev-token" }] = googleUserId;

    const int64_t steamUserId = self->m_nextUserId++;
    StoredUser steamUser;
    steamUser.User = MakeUser(steamUserId, "steam.qa@infinity.local", "SteamTester");
    steamUser.Identities.push_back(MakeIdentity(steamUserId, "steam", "steam-dev-ticket"));
    self->m_usersById.emplace(steamUserId, steamUser);
    self->m_userIdByEmail[steamUser.User.Email] = steamUserId;
    self->m_userIdByIdentity[{ "steam", "steam-dev-ticket" }] = steamUserId;
}

bool DBConnector::EmailExists(const std::string& email) const
{
    return m_userIdByEmail.find(email) != m_userIdByEmail.end();
}

bool DBConnector::NicknameExists(const std::string& nickname) const
{
    return std::any_of(m_usersById.begin(),
                       m_usersById.end(),
                       [&](const auto& entry)
                       {
                           return entry.second.User.Nickname == nickname;
                       });
}

std::optional<DbUser> DBConnector::FindUserByIdentity(const std::string& provider,
                                                      const std::string& providerUserId,
                                                      bool verifyPassword,
                                                      const std::string& plainPassword) const
{
    const auto identityIt = m_userIdByIdentity.find({ provider, providerUserId });
    if (identityIt == m_userIdByIdentity.end())
    {
        return std::nullopt;
    }

    const auto userIt = m_usersById.find(identityIt->second);
    if (userIt == m_usersById.end())
    {
        return std::nullopt;
    }

    if (verifyPassword)
    {
        const auto identity = std::find_if(userIt->second.Identities.begin(),
                                           userIt->second.Identities.end(),
                                           [&](const DbIdentity& item)
                                           {
                                               return item.Provider == provider
                                                   && item.ProviderUserId == providerUserId;
                                           });
        if (identity == userIt->second.Identities.end()
            || !PasswordHasher::VerifyForDevelopment(plainPassword, identity->PasswordHash))
        {
            return std::nullopt;
        }
    }

    return userIt->second.User;
}

std::optional<DbUser> DBConnector::FindUserByEmail(const std::string& email) const
{
    const auto it = m_userIdByEmail.find(email);
    if (it == m_userIdByEmail.end())
    {
        return std::nullopt;
    }

    const auto userIt = m_usersById.find(it->second);
    if (userIt == m_usersById.end())
    {
        return std::nullopt;
    }

    return userIt->second.User;
}

DbPlayerAggregate DBConnector::BuildAggregateForUser(int64_t userId) const
{
    DbPlayerAggregate aggregate;
    aggregate.UserId = userId;

    for (const DbMatch& match : m_matches)
    {
        for (const DbMatchPlayer& player : match.Players)
        {
            if (player.UserId != userId)
            {
                continue;
            }

            ++aggregate.TotalMatches;
            if (player.Result == "WIN")
            {
                ++aggregate.TotalWins;
            }

            aggregate.TotalKills += player.Kills;
            aggregate.TotalDeaths += player.Deaths;
            aggregate.TotalAssists += player.Assists;
            aggregate.TotalDamageDealt += player.DamageDealt;
        }
    }

    return aggregate;
}
