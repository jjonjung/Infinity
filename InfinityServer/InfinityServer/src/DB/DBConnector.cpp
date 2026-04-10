#include "DB/DBConnector.h"

#include "Shared/Security/PasswordHasher.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#if __has_include(<jdbc/mysql_driver.h>) && __has_include(<jdbc/mysql_connection.h>) && __has_include(<jdbc/cppconn/prepared_statement.h>)
#define INFINITY_HAS_MYSQLCPP 1
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#else
#define INFINITY_HAS_MYSQLCPP 0
namespace sql
{
class Connection
{
};
class Driver
{
};
class SQLException
{
};
}
#endif

namespace
{
constexpr const char* kCallUserExistsByEmail = "CALL sp_user_exists_by_email(?)";
constexpr const char* kCallUserExistsByNickname = "CALL sp_user_exists_by_nickname(?)";
constexpr const char* kCallCreateLocalUser = "CALL sp_create_local_user(?, ?, ?)";
constexpr const char* kCallAuthenticateLocalUser = "CALL sp_authenticate_local_user(?)";
constexpr const char* kCallAuthenticateSocialUser = "CALL sp_authenticate_social_user(?, ?)";
constexpr const char* kCallSaveUserSession = "CALL sp_save_user_session(?, ?, ?, ?)";
constexpr const char* kCallUpsertMatchHeader = "CALL sp_upsert_match_header(?, ?)";
constexpr const char* kCallDeleteMatchPlayers = "CALL sp_delete_match_players(?)";
constexpr const char* kCallInsertMatchPlayer = "CALL sp_insert_match_player(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
constexpr const char* kCallGetPlayerAggregate = "CALL sp_get_player_aggregate(?)";
constexpr const char* kCallEnsureSocialSeedUser = "CALL sp_ensure_social_seed_user(?, ?, ?, ?)";

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

std::string BuildMysqlEndpoint(const std::string& host, int port)
{
    std::ostringstream stream;
    stream << "tcp://" << host << ":" << port;
    return stream.str();
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
    m_connected = false;
    m_useMysqlBackend = false;

#if INFINITY_HAS_MYSQLCPP
    try
    {
        m_driver = sql::mysql::get_mysql_driver_instance();
        auto connection = OpenConnection();
        if (connection)
        {
            m_useMysqlBackend = true;
            EnsureSchema();
            SeedDevelopmentData();
            m_connected = true;
            std::cout << "[DB] mysqlcppconn8 backend connected to " << m_host << ":" << m_port << "\n";
            return true;
        }
    }
    catch (const sql::SQLException&)
    {
        std::cout << "[DB] mysqlcppconn8 connection failed, falling back to in-memory store\n";
    }
#endif

    SeedDevelopmentData();
    std::cout << "[DB] initialized in-memory persistence for mysql schema '" << m_database << "'\n";
    return false;
}

void DBConnector::Disconnect()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connected = false;
    m_useMysqlBackend = false;
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

#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto createUser = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallCreateLocalUser));
            createUser->setString(1, email);
            createUser->setString(2, nickname);
            createUser->setString(3, passwordHash);
            createUser->execute();

            auto userResult = std::unique_ptr<sql::ResultSet>(createUser->getResultSet());
            if (!userResult->next())
            {
                return std::nullopt;
            }

            DbUser user;
            user.UserId = userResult->getInt64("id");
            user.Email = userResult->getString("email");
            user.Nickname = userResult->getString("nickname");
            user.Status = userResult->getString("status");
            return user;
        }
        catch (const sql::SQLException&)
        {
            return std::nullopt;
        }
    }
#endif

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

#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto stmt = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallSaveUserSession));
            stmt->setInt64(1, session.UserId);
            stmt->setString(2, session.RefreshTokenHash);
            stmt->setString(3, "dev-device");
            stmt->setString(4, "127.0.0.1");
            stmt->executeUpdate();
            return true;
        }
        catch (const sql::SQLException&)
        {
            return false;
        }
    }
#endif

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

#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();

            auto matchSelect = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallUpsertMatchHeader));
            matchSelect->setString(1, match.ExternalMatchId);
            matchSelect->setString(2, match.WinnerTeam);
            matchSelect->execute();
            auto matchResult = std::unique_ptr<sql::ResultSet>(matchSelect->getResultSet());
            if (!matchResult || !matchResult->next())
            {
                return false;
            }

            const int64_t matchId = matchResult->getInt64("match_id");

            auto deletePlayers = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallDeleteMatchPlayers));
            deletePlayers->setInt64(1, matchId);
            deletePlayers->executeUpdate();

            auto insertPlayer = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallInsertMatchPlayer));

            for (const DbMatchPlayer& player : match.Players)
            {
                insertPlayer->setInt64(1, matchId);
                insertPlayer->setInt64(2, player.UserId);
                insertPlayer->setString(3, player.Team);
                insertPlayer->setString(4, player.CharacterName);
                insertPlayer->setInt(5, player.Kills);
                insertPlayer->setInt(6, player.Deaths);
                insertPlayer->setInt(7, player.Assists);
                insertPlayer->setInt(8, player.DamageDealt);
                insertPlayer->setInt(9, player.DamageTaken);
                insertPlayer->setInt(10, player.Score);
                insertPlayer->setString(11, player.Result);
                insertPlayer->executeUpdate();
            }

            return true;
        }
        catch (const sql::SQLException&)
        {
            return false;
        }
    }
#endif

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

#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto stmt = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallGetPlayerAggregate));
            stmt->setInt64(1, userId);
            stmt->execute();
            auto result = std::unique_ptr<sql::ResultSet>(stmt->getResultSet());
            if (!result->next())
            {
                return std::nullopt;
            }

            DbPlayerAggregate aggregate;
            aggregate.UserId = result->getInt64("user_id");
            aggregate.TotalMatches = result->getInt("total_matches");
            aggregate.TotalWins = result->getInt("total_wins");
            aggregate.TotalKills = result->getInt("total_kills");
            aggregate.TotalDeaths = result->getInt("total_deaths");
            aggregate.TotalAssists = result->getInt("total_assists");
            aggregate.TotalDamageDealt = result->getInt("total_damage_dealt");
            return aggregate;
        }
        catch (const sql::SQLException&)
        {
            return std::nullopt;
        }
    }
#endif

    if (m_usersById.find(userId) == m_usersById.end())
    {
        return std::nullopt;
    }

    return BuildAggregateForUser(userId);
}

std::unique_ptr<sql::Connection> DBConnector::OpenConnection() const
{
#if INFINITY_HAS_MYSQLCPP
    if (m_driver == nullptr)
    {
        return {};
    }

    auto connection = std::unique_ptr<sql::Connection>(
        m_driver->connect(BuildMysqlEndpoint(m_host, m_port), m_user, m_password));
    connection->setSchema(m_database);
    return connection;
#else
    return {};
#endif
}

void DBConnector::EnsureSchema() const
{
#if INFINITY_HAS_MYSQLCPP
    if (!m_useMysqlBackend)
    {
        return;
    }

    try
    {
        auto connection = OpenConnection();
        auto statement = std::unique_ptr<sql::Statement>(connection->createStatement());

        statement->execute(
            "CREATE TABLE IF NOT EXISTS users ("
            "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
            "email VARCHAR(255) NULL,"
            "nickname VARCHAR(50) NOT NULL,"
            "status VARCHAR(20) NOT NULL DEFAULT 'ACTIVE',"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
            "UNIQUE KEY uq_users_email (email),"
            "UNIQUE KEY uq_users_nickname (nickname))");

        statement->execute(
            "CREATE TABLE IF NOT EXISTS user_identities ("
            "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
            "user_id BIGINT NOT NULL,"
            "provider VARCHAR(20) NOT NULL,"
            "provider_user_id VARCHAR(191) NOT NULL,"
            "password_hash VARCHAR(255) NULL,"
            "email_at_provider VARCHAR(255) NULL,"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "UNIQUE KEY uq_provider_identity (provider, provider_user_id),"
            "CONSTRAINT fk_user_identities_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE)");

        statement->execute(
            "CREATE TABLE IF NOT EXISTS user_sessions ("
            "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
            "user_id BIGINT NOT NULL,"
            "refresh_token_hash VARCHAR(255) NOT NULL,"
            "device_id VARCHAR(128) NULL,"
            "ip_address VARCHAR(64) NULL,"
            "expires_at DATETIME NOT NULL,"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "revoked_at DATETIME NULL,"
            "CONSTRAINT fk_user_sessions_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE)");

        statement->execute(
            "CREATE TABLE IF NOT EXISTS matches ("
            "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
            "game_mode VARCHAR(50) NOT NULL,"
            "map_name VARCHAR(100) NOT NULL,"
            "started_at DATETIME NOT NULL,"
            "ended_at DATETIME NULL,"
            "winner_team VARCHAR(20) NULL,"
            "status VARCHAR(20) NOT NULL DEFAULT 'IN_PROGRESS',"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)");

        statement->execute(
            "CREATE TABLE IF NOT EXISTS match_players ("
            "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
            "match_id BIGINT NOT NULL,"
            "user_id BIGINT NOT NULL,"
            "team VARCHAR(20) NOT NULL,"
            "character_name VARCHAR(100) NOT NULL,"
            "kills INT NOT NULL DEFAULT 0,"
            "deaths INT NOT NULL DEFAULT 0,"
            "assists INT NOT NULL DEFAULT 0,"
            "damage_dealt INT NOT NULL DEFAULT 0,"
            "damage_taken INT NOT NULL DEFAULT 0,"
            "score INT NOT NULL DEFAULT 0,"
            "result VARCHAR(20) NULL,"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "UNIQUE KEY uq_match_user (match_id, user_id),"
            "CONSTRAINT fk_match_players_match FOREIGN KEY (match_id) REFERENCES matches(id) ON DELETE CASCADE,"
            "CONSTRAINT fk_match_players_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE)");
    }
    catch (const sql::SQLException&)
    {
    }
#endif
}

void DBConnector::SeedDevelopmentData() const
{
#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto ensureSeed = [&](const std::string& provider,
                                  const std::string& token,
                                  const std::string& email,
                                  const std::string& nickname)
            {
                auto stmt = std::unique_ptr<sql::PreparedStatement>(
                    connection->prepareStatement(kCallEnsureSocialSeedUser));
                stmt->setString(1, provider);
                stmt->setString(2, token);
                stmt->setString(3, email);
                stmt->setString(4, nickname);
                stmt->executeUpdate();
            };

            ensureSeed("google", "google-dev-token", "google.qa@infinity.local", "GoogleTester");
            ensureSeed("steam", "steam-dev-ticket", "steam.qa@infinity.local", "SteamTester");
        }
        catch (const sql::SQLException&)
        {
        }
        return;
    }
#endif

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
#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto stmt = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallUserExistsByEmail));
            stmt->setString(1, email);
            stmt->execute();
            auto result = std::unique_ptr<sql::ResultSet>(stmt->getResultSet());
            return result && result->next() && result->getInt("exists_flag") != 0;
        }
        catch (const sql::SQLException&)
        {
            return false;
        }
    }
#endif

    return m_userIdByEmail.find(email) != m_userIdByEmail.end();
}

bool DBConnector::NicknameExists(const std::string& nickname) const
{
#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto stmt = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(kCallUserExistsByNickname));
            stmt->setString(1, nickname);
            stmt->execute();
            auto result = std::unique_ptr<sql::ResultSet>(stmt->getResultSet());
            return result && result->next() && result->getInt("exists_flag") != 0;
        }
        catch (const sql::SQLException&)
        {
            return false;
        }
    }
#endif

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
#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto stmt = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement(verifyPassword ? kCallAuthenticateLocalUser : kCallAuthenticateSocialUser));
            stmt->setString(1, providerUserId);
            if (!verifyPassword)
            {
                stmt->setString(1, provider);
                stmt->setString(2, providerUserId);
            }
            stmt->execute();
            auto result = std::unique_ptr<sql::ResultSet>(stmt->getResultSet());
            if (!result->next())
            {
                return std::nullopt;
            }

            if (verifyPassword
                && !PasswordHasher::VerifyForDevelopment(plainPassword, result->getString("password_hash")))
            {
                return std::nullopt;
            }

            DbUser user;
            user.UserId = result->getInt64("id");
            user.Email = result->isNull("email") ? "" : result->getString("email");
            user.Nickname = result->getString("nickname");
            user.Status = result->getString("status");
            return user;
        }
        catch (const sql::SQLException&)
        {
            return std::nullopt;
        }
    }
#endif

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
#if INFINITY_HAS_MYSQLCPP
    if (m_useMysqlBackend)
    {
        try
        {
            auto connection = OpenConnection();
            auto stmt = std::unique_ptr<sql::PreparedStatement>(
                connection->prepareStatement("SELECT id, email, nickname, status FROM users WHERE email = ?"));
            stmt->setString(1, email);
            auto result = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!result->next())
            {
                return std::nullopt;
            }

            DbUser user;
            user.UserId = result->getInt64("id");
            user.Email = result->getString("email");
            user.Nickname = result->getString("nickname");
            user.Status = result->getString("status");
            return user;
        }
        catch (const sql::SQLException&)
        {
            return std::nullopt;
        }
    }
#endif

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
