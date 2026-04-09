#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <unordered_map>

class RedisCache
{
public:
    bool Connect(const std::string& host, int port);
    void Disconnect();
    bool IsConnected() const;

    std::string BuildSessionKey(int64_t userId) const;
    std::string BuildLeaderboardKey(const std::string& seasonId) const;
    std::string BuildMatchSnapshotKey(const std::string& matchId) const;

    void StoreGameSessionToken(const std::string& token, int64_t userId);
    std::optional<int64_t> FindUserIdByGameSessionToken(const std::string& token) const;
    void CacheUserSession(int64_t userId, const std::string& refreshToken, const std::string& gameSessionToken);
    void CacheMatchSnapshot(const std::string& matchId, const std::string& summary);
    void CacheLeaderboardEntry(const std::string& seasonId, int64_t userId, int score);
    int GetCachedSessionCount() const;
    int GetCachedMatchCount() const;
    int GetLeaderboardEntryCount(const std::string& seasonId) const;

private:
    bool m_connected = false;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, int64_t> m_gameSessions;
    std::unordered_map<std::string, std::string> m_cachedSessions;
    std::unordered_map<std::string, std::string> m_matchSnapshots;
    std::unordered_map<std::string, std::unordered_map<int64_t, int>> m_leaderboards;
};
