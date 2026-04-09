#include "../include/Tcp/TcpClient.h"
#include "../include/Tcp/TcpServer.h"

#include <cassert>
#include <chrono>
#include <thread>

using namespace TcpUdpSimulator;

int main()
{
    TcpServer server;
    TcpClient client;

    assert(server.Start(7001));
    assert(client.Connect("127.0.0.1", 7001));

    for (int attempt = 0; attempt < 20 && !server.HasClient(); ++attempt)
    {
        server.TickAccept();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(server.HasClient());

    const char payload[] = "PING";
    assert(client.Send(payload, sizeof(payload)));

    int received = 0;
    for (int attempt = 0; attempt < 20 && received == 0; ++attempt)
    {
        server.TickReceive();
        received = server.GetLastReceiveSize();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(received == sizeof(payload));

    client.Disconnect();
    server.Stop();
    return 0;
}
