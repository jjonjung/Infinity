#pragma once

#include "Infrastructure/Cache/RedisCache.h"

#include <string>
#include <vector>

struct MonitoringNodeStatus
{
    std::string Name;
    bool Healthy = false;
};

struct MonitoringSnapshot
{
    std::vector<MonitoringNodeStatus> Nodes;
    int ActiveMatchCount = 0;
    int ConnectedSessionCount = 0;
};

class AdminMonitoringService
{
public:
    explicit AdminMonitoringService(const RedisCache& redisCache);

    MonitoringSnapshot BuildSnapshot() const;

private:
    const RedisCache& m_redisCache;
};
