#include "Admin/Application/AdminMonitoringService.h"

#include "DB/DBConnector.h"
#include "Network/Session.h"

AdminMonitoringService::AdminMonitoringService(const RedisCache& redisCache)
    : m_redisCache(redisCache)
{
}

MonitoringSnapshot AdminMonitoringService::BuildSnapshot() const
{
    MonitoringSnapshot snapshot;
    snapshot.Nodes.push_back({ "auth-api", true });
    snapshot.Nodes.push_back({ "dedicated-game-server", true });
    snapshot.Nodes.push_back({ "redis", m_redisCache.IsConnected() });
    snapshot.Nodes.push_back({ "mysql", DBConnector::Get().IsConnected() });
    snapshot.ActiveMatchCount = m_redisCache.GetCachedMatchCount();
    snapshot.ConnectedSessionCount = Session::GetActiveSessionCount();
    snapshot.CachedLeaderboardEntryCount = m_redisCache.GetLeaderboardEntryCount("current");
    return snapshot;
}
