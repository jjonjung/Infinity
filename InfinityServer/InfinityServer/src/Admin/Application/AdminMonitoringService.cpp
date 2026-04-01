#include "Admin/Application/AdminMonitoringService.h"

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
    snapshot.Nodes.push_back({ "mysql", true });
    return snapshot;
}
