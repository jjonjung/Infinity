#include "Infrastructure/Cache/RedisCache.h"

bool RedisCache::Connect(const std::string& host, int port)
{
    m_connected = !host.empty() && port > 0;
    return m_connected;
}

void RedisCache::Disconnect()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connected = false;
    m_gameSessions.clear();
}

bool RedisCache::IsConnected() const
{
    return m_connected;
}

std::string RedisCache::BuildSessionKey(int64_t userId) const
{
    return "session:user:" + std::to_string(userId);
}

std::string RedisCache::BuildLeaderboardKey(const std::string& seasonId) const
{
    return "leaderboard:season:" + seasonId;
}

std::string RedisCache::BuildMatchSnapshotKey(const std::string& matchId) const
{
    return "match:snapshot:" + matchId;
}

void RedisCache::StoreGameSessionToken(const std::string& token, int64_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_gameSessions[token] = userId;
}

std::optional<int64_t> RedisCache::FindUserIdByGameSessionToken(const std::string& token) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_gameSessions.find(token);
    if (it == m_gameSessions.end())
    {
        return std::nullopt;
    }

    return it->second;
}
