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
    m_cachedSessions.clear();
    m_matchSnapshots.clear();
    m_leaderboards.clear();
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
    m_cachedSessions[BuildSessionKey(userId)] = token;
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

void RedisCache::CacheUserSession(int64_t userId, const std::string& refreshToken, const std::string& gameSessionToken)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cachedSessions[BuildSessionKey(userId)] = refreshToken + "|" + gameSessionToken;
    m_gameSessions[gameSessionToken] = userId;
}

void RedisCache::CacheMatchSnapshot(const std::string& matchId, const std::string& summary)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_matchSnapshots[BuildMatchSnapshotKey(matchId)] = summary;
}

void RedisCache::CacheLeaderboardEntry(const std::string& seasonId, int64_t userId, int score)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_leaderboards[BuildLeaderboardKey(seasonId)][userId] = score;
}

int RedisCache::GetCachedSessionCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(m_cachedSessions.size());
}

int RedisCache::GetCachedMatchCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(m_matchSnapshots.size());
}

int RedisCache::GetLeaderboardEntryCount(const std::string& seasonId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_leaderboards.find(BuildLeaderboardKey(seasonId));
    if (it == m_leaderboards.end())
    {
        return 0;
    }

    return static_cast<int>(it->second.size());
}
