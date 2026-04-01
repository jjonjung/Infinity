#include "Bootstrap/ServerRuntime.h"
#include "Network/Server.h"
#include "Shared/Config/ServerConfig.h"
#include "Shared/Logging/Logger.h"

#include <windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const ServerConfig config = ServerConfigLoader::LoadDefaults();
    ServerRuntime::Initialize(config);
    Logger::Write(LogLevel::Info, "bootstrap", "InfinityServer starting");

    Server server(config.Game.DedicatedPort);

    if (!server.Init())
    {
        Logger::Write(LogLevel::Error, "bootstrap", "failed to initialize dedicated server");
        return 1;
    }

    Logger::Write(LogLevel::Info, "bootstrap", "dedicated server initialized");
    server.Run();

    return 0;
}
