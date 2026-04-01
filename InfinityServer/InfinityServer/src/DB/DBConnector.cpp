#include "DB/DBConnector.h"

#include "Shared/Security/PasswordHasher.h"

#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>

#include <iostream>
#include <sstream>

namespace
{
std::string BuildConnectionUri(const std::string& host, int port)
{
    std::ostringstream stream;
    stream << "tcp://" << host << ":" << port;
    return stream.str();
}

DbUser ReadUser(sql::ResultSet& result)
{
    DbUser user;
    user.UserId = result.getInt64("id");
    user.Email = result.isNull("email") ? "" : result.getString("email");
    user.Nickname = result.getString("nickname");
    user.Status = result.getString("status");
    return user;
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

    try
    {
        m_host = host;
        m_port = port;
        m_user = user;
        m_password = password;
        m_database = database;
        m_driver = sql::mysql::get_driver_instance();

        auto connection = OpenConnection();
        connection->setSchema(m_database);
        m_connected = true;

        EnsureSchema();
        SeedDevelopmentData();

        std::cout << "[DB] mysql connector initialized\n";
        return true;
    }
    catch (const sql::SQLException& ex)
    {
        m_connected = false;
        std::cout << "[DB] mysql connector initialization failed: " << ex.what() << "\n";
        return false;
    }
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
    if (!m_connected || email.empty() || nickname.empty() || passwordHash.empty())
    {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        if (EmailExists(email) || NicknameExists(nickname))
        {
            return std::nullopt;
        }

        auto connection = OpenConnection();
        connection->setSchema(m_database);
        connection->setAutoCommit(false);

        auto insertUser = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "INSERT INTO users (email, nickname, status) VALUES (?, ?, 'ACTIVE')"));
        insertUser->setString(1, email);
        insertUser->setString(2, nickname);
        insertUser->executeUpdate();

        auto fetchId = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement("SELECT LAST_INSERT_ID() AS id"));
        auto idResult = std::unique_ptr<sql::ResultSet>(fetchId->executeQuery());
        if (!idResult->next())
        {
            connection->rollback();
            return std::nullopt;
        }

        const auto userId = idResult->getInt64("id");

        auto insertIdentity = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "INSERT INTO user_identities (user_id, provider, provider_user_id, password_hash, email_at_provider) "
                "VALUES (?, 'local', ?, ?, ?)"));
        insertIdentity->setInt64(1, userId);
        insertIdentity->setString(2, email);
        insertIdentity->setString(3, passwordHash);
        insertIdentity->setString(4, email);
        insertIdentity->executeUpdate();

        connection->commit();

        return DbUser{ userId, email, nickname, "ACTIVE" };
    }
    catch (const sql::SQLException&)
    {
        return std::nullopt;
    }
}

std::optional<DbUser> DBConnector::AuthenticateLocalUser(const std::string& email,
                                                         const std::string& plainPassword) const
{
    return FindUserByIdentity("local", email, true, plainPassword);
}

std::optional<DbUser> DBConnector::AuthenticateSocialUser(const std::string& provider,
                                                          const std::string& providerUserId) const
{
    return FindUserByIdentity(provider, providerUserId, false, "");
}

bool DBConnector::SaveSession(const DbSession& session)
{
    if (!m_connected || session.UserId <= 0 || session.RefreshTokenHash.empty())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        auto connection = OpenConnection();
        connection->setSchema(m_database);

        auto insertSession = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "INSERT INTO user_sessions (user_id, refresh_token_hash, device_id, ip_address, expires_at) "
                "VALUES (?, ?, ?, ?, DATE_ADD(NOW(), INTERVAL 30 DAY))"));
        insertSession->setInt64(1, session.UserId);
        insertSession->setString(2, PasswordHasher::HashForDevelopment(session.RefreshTokenHash));
        insertSession->setString(3, session.GameSessionToken);
        insertSession->setString(4, "127.0.0.1");
        return insertSession->executeUpdate() == 1;
    }
    catch (const sql::SQLException&)
    {
        return false;
    }
}

bool DBConnector::PersistMatch(const DbMatch& match)
{
    if (!m_connected || match.Players.empty())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        auto connection = OpenConnection();
        connection->setSchema(m_database);
        connection->setAutoCommit(false);

        auto insertMatch = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "INSERT INTO matches (game_mode, map_name, started_at, ended_at, winner_team, status) "
                "VALUES (?, ?, NOW(), NOW(), ?, 'FINISHED')"));
        insertMatch->setString(1, "DEFAULT");
        insertMatch->setString(2, match.ExternalMatchId.empty() ? "UNKNOWN_MATCH" : match.ExternalMatchId);
        insertMatch->setString(3, match.WinnerTeam.empty() ? "" : match.WinnerTeam);
        insertMatch->executeUpdate();

        auto fetchId = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement("SELECT LAST_INSERT_ID() AS id"));
        auto idResult = std::unique_ptr<sql::ResultSet>(fetchId->executeQuery());
        if (!idResult->next())
        {
            connection->rollback();
            return false;
        }

        const auto matchId = idResult->getInt64("id");

        auto insertPlayer = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "INSERT INTO match_players "
                "(match_id, user_id, team, character_name, kills, deaths, assists, damage_dealt, damage_taken, score, result) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

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
            insertPlayer->setString(11, player.Result.empty() ? "UNKNOWN" : player.Result);
            insertPlayer->executeUpdate();
        }

        connection->commit();
        return true;
    }
    catch (const sql::SQLException&)
    {
        return false;
    }
}

std::optional<DbPlayerAggregate> DBConnector::GetAggregateForUser(int64_t userId) const
{
    if (!m_connected || userId <= 0)
    {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        auto connection = OpenConnection();
        connection->setSchema(m_database);

        auto query = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "SELECT user_id, "
                "COUNT(*) AS total_matches, "
                "SUM(CASE WHEN result = 'WIN' THEN 1 ELSE 0 END) AS total_wins, "
                "COALESCE(SUM(kills), 0) AS total_kills, "
                "COALESCE(SUM(deaths), 0) AS total_deaths, "
                "COALESCE(SUM(assists), 0) AS total_assists, "
                "COALESCE(SUM(damage_dealt), 0) AS total_damage_dealt "
                "FROM match_players WHERE user_id = ? GROUP BY user_id"));
        query->setInt64(1, userId);

        auto result = std::unique_ptr<sql::ResultSet>(query->executeQuery());
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

std::unique_ptr<sql::Connection> DBConnector::OpenConnection() const
{
    auto connection = std::unique_ptr<sql::Connection>(
        m_driver->connect(BuildConnectionUri(m_host, m_port), m_user, m_password));
    return connection;
}

void DBConnector::EnsureSchema() const
{
    auto connection = OpenConnection();
    auto statement = std::unique_ptr<sql::Statement>(connection->createStatement());

    statement->execute("CREATE DATABASE IF NOT EXISTS `" + m_database + "`");
    connection->setSchema(m_database);

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

void DBConnector::SeedDevelopmentData() const
{
    auto connection = OpenConnection();
    connection->setSchema(m_database);
    connection->setAutoCommit(false);

    struct SeedUser
    {
        int64_t Id;
        const char* Email;
        const char* Nickname;
        const char* Provider;
        const char* ProviderUserId;
        std::string PasswordHash;
    };

    const std::vector<SeedUser> seedUsers = {
        { 1001, "tester@infinity.local", "LocalTester", "local", "tester@infinity.local", PasswordHasher::HashForDevelopment("pass1234") },
        { 1002, "operator@infinity.local", "Operator", "local", "operator@infinity.local", PasswordHasher::HashForDevelopment("admin1234") },
        { 2001, "google-tester@infinity.local", "GoogleTester", "google", "google-dev-token", "" },
        { 3001, "steam-tester@infinity.local", "SteamTester", "steam", "steam-dev-ticket", "" }
    };

    auto insertUser = std::unique_ptr<sql::PreparedStatement>(
        connection->prepareStatement(
            "INSERT INTO users (id, email, nickname, status) VALUES (?, ?, ?, 'ACTIVE') "
            "ON DUPLICATE KEY UPDATE email = VALUES(email), nickname = VALUES(nickname), status = VALUES(status)"));

    auto insertIdentity = std::unique_ptr<sql::PreparedStatement>(
        connection->prepareStatement(
            "INSERT INTO user_identities (user_id, provider, provider_user_id, password_hash, email_at_provider) "
            "VALUES (?, ?, ?, ?, ?) "
            "ON DUPLICATE KEY UPDATE password_hash = VALUES(password_hash), email_at_provider = VALUES(email_at_provider)"));

    for (const SeedUser& seedUser : seedUsers)
    {
        insertUser->setInt64(1, seedUser.Id);
        insertUser->setString(2, seedUser.Email);
        insertUser->setString(3, seedUser.Nickname);
        insertUser->executeUpdate();

        insertIdentity->setInt64(1, seedUser.Id);
        insertIdentity->setString(2, seedUser.Provider);
        insertIdentity->setString(3, seedUser.ProviderUserId);
        insertIdentity->setString(4, seedUser.PasswordHash);
        insertIdentity->setString(5, seedUser.Email);
        insertIdentity->executeUpdate();
    }

    connection->commit();
}

bool DBConnector::EmailExists(const std::string& email) const
{
    auto connection = OpenConnection();
    connection->setSchema(m_database);

    auto query = std::unique_ptr<sql::PreparedStatement>(
        connection->prepareStatement("SELECT 1 FROM users WHERE email = ? LIMIT 1"));
    query->setString(1, email);
    auto result = std::unique_ptr<sql::ResultSet>(query->executeQuery());
    return result->next();
}

bool DBConnector::NicknameExists(const std::string& nickname) const
{
    auto connection = OpenConnection();
    connection->setSchema(m_database);

    auto query = std::unique_ptr<sql::PreparedStatement>(
        connection->prepareStatement("SELECT 1 FROM users WHERE nickname = ? LIMIT 1"));
    query->setString(1, nickname);
    auto result = std::unique_ptr<sql::ResultSet>(query->executeQuery());
    return result->next();
}

std::optional<DbUser> DBConnector::FindUserByIdentity(const std::string& provider,
                                                      const std::string& providerUserId,
                                                      bool verifyPassword,
                                                      const std::string& plainPassword) const
{
    if (!m_connected || provider.empty() || providerUserId.empty())
    {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        auto connection = OpenConnection();
        connection->setSchema(m_database);

        auto query = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "SELECT u.id, u.email, u.nickname, u.status, ui.password_hash "
                "FROM user_identities ui "
                "INNER JOIN users u ON u.id = ui.user_id "
                "WHERE ui.provider = ? AND ui.provider_user_id = ? LIMIT 1"));
        query->setString(1, provider);
        query->setString(2, providerUserId);

        auto result = std::unique_ptr<sql::ResultSet>(query->executeQuery());
        if (!result->next())
        {
            return std::nullopt;
        }

        const std::string passwordHash = result->isNull("password_hash") ? "" : result->getString("password_hash");
        if (verifyPassword && !PasswordHasher::VerifyForDevelopment(plainPassword, passwordHash))
        {
            return std::nullopt;
        }

        return ReadUser(*result);
    }
    catch (const sql::SQLException&)
    {
        return std::nullopt;
    }
}
