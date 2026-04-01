#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
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

private:
    bool m_connected = false;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, int64_t> m_gameSessions;
};
