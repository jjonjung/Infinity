#include "DB/DBConnector.h"

#include <iostream>

namespace sql
{
class Connection
{
};
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
    std::cout << "[DB] mysql connector is not configured; database features are disabled\n";
    return false;
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
    return std::nullopt;
}

std::optional<DbUser> DBConnector::AuthenticateLocalUser(const std::string& email,
                                                         const std::string& plainPassword) const
{
    return std::nullopt;
}

std::optional<DbUser> DBConnector::AuthenticateSocialUser(const std::string& provider,
                                                          const std::string& providerUserId) const
{
    return std::nullopt;
}

bool DBConnector::SaveSession(const DbSession& session)
{
    return false;
}

bool DBConnector::PersistMatch(const DbMatch& match)
{
    return false;
}

std::optional<DbPlayerAggregate> DBConnector::GetAggregateForUser(int64_t userId) const
{
    return std::nullopt;
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
}

bool DBConnector::EmailExists(const std::string& email) const
{
    return false;
}

bool DBConnector::NicknameExists(const std::string& nickname) const
{
    return false;
}

std::optional<DbUser> DBConnector::FindUserByIdentity(const std::string& provider,
                                                      const std::string& providerUserId,
                                                      bool verifyPassword,
                                                      const std::string& plainPassword) const
{
    return std::nullopt;
}
