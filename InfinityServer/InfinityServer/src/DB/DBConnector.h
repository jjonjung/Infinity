#pragma once

#include <cstdint>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <string>
#include <vector>

namespace sql
{
class Connection;
class Driver;
}

struct DbUser
{
    int64_t UserId = 0;
    std::string Email;
    std::string Nickname;
    std::string Status;
};

struct DbIdentity
{
    int64_t UserId = 0;
    std::string Provider;
    std::string ProviderUserId;
    std::string PasswordHash;
};

struct DbSession
{
    int64_t UserId = 0;
    std::string RefreshTokenHash;
    std::string GameSessionToken;
};

struct DbMatchPlayer
{
    int64_t UserId = 0;
    std::string Team;
    std::string CharacterName;
    int Kills = 0;
    int Deaths = 0;
    int Assists = 0;
    int DamageDealt = 0;
    int DamageTaken = 0;
    int Score = 0;
    std::string Result;
};

struct DbMatch
{
    int64_t MatchId = 0;
    std::string ExternalMatchId;
    std::string WinnerTeam;
    int PlayerCount = 0;
    std::vector<DbMatchPlayer> Players;
};

struct DbPlayerAggregate
{
    int64_t UserId = 0;
    int TotalMatches = 0;
    int TotalWins = 0;
    int TotalKills = 0;
    int TotalDeaths = 0;
    int TotalAssists = 0;
    int TotalDamageDealt = 0;
};

class DBConnector
{
public:
    static DBConnector& Get();

    bool Connect(const std::string& host, int port,
                 const std::string& user,
                 const std::string& password,
                 const std::string& database);

    void Disconnect();
    bool IsConnected() const { return m_connected; }

    std::optional<DbUser> CreateLocalUser(const std::string& email,
                                          const std::string& nickname,
                                          const std::string& passwordHash);
    std::optional<DbUser> AuthenticateLocalUser(const std::string& email,
                                                const std::string& plainPassword) const;
    std::optional<DbUser> AuthenticateSocialUser(const std::string& provider,
                                                 const std::string& providerUserId) const;
    bool SaveSession(const DbSession& session);
    bool PersistMatch(const DbMatch& match);
    std::optional<DbPlayerAggregate> GetAggregateForUser(int64_t userId) const;

private:
    struct StoredUser
    {
        DbUser User;
        std::vector<DbIdentity> Identities;
    };

    struct StoredSession
    {
        DbSession Session;
        std::chrono::system_clock::time_point CreatedAt;
    };

    DBConnector();
    ~DBConnector() = default;
    DBConnector(const DBConnector&) = delete;
    DBConnector& operator=(const DBConnector&) = delete;

    std::unique_ptr<sql::Connection> OpenConnection() const;
    void EnsureSchema() const;
    void SeedDevelopmentData() const;
    bool EmailExists(const std::string& email) const;
    bool NicknameExists(const std::string& nickname) const;
    std::optional<DbUser> FindUserByIdentity(const std::string& provider,
                                             const std::string& providerUserId,
                                             bool verifyPassword,
                                             const std::string& plainPassword) const;
    std::optional<DbUser> FindUserByEmail(const std::string& email) const;
    DbPlayerAggregate BuildAggregateForUser(int64_t userId) const;

    bool m_connected = false;
    std::string m_host;
    int m_port = 0;
    std::string m_user;
    std::string m_password;
    std::string m_database;
    bool m_useMysqlBackend = false;
    sql::Driver* m_driver = nullptr;
    int64_t m_nextUserId = 1000;
    int64_t m_nextMatchId = 1;
    std::unordered_map<int64_t, StoredUser> m_usersById;
    std::unordered_map<std::string, int64_t> m_userIdByEmail;
    std::map<std::pair<std::string, std::string>, int64_t> m_userIdByIdentity;
    std::vector<StoredSession> m_sessions;
    std::vector<DbMatch> m_matches;
    mutable std::mutex m_mutex;
};
