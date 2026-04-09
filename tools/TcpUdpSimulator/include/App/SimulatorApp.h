#pragma once

#include "../Tcp/TcpClient.h"
#include "../Tcp/TcpServer.h"
#include "../Udp/UdpBroadcaster.h"
#include "../Udp/UdpListener.h"

namespace TcpUdpSimulator
{

class SimulatorApp
{
public:
    void RunServerMode(int tcpPort, int udpPort);
    void RunClientMode(const char* host, int tcpPort, int udpPort);

private:
    TcpServer tcpServer_;
    TcpClient tcpClient_;
    UdpBroadcaster udpBroadcaster_;
    UdpListener udpListener_;
};

} // namespace TcpUdpSimulator
