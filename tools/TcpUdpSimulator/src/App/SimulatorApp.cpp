#include "../../include/App/SimulatorApp.h"

namespace TcpUdpSimulator
{

void SimulatorApp::RunServerMode(int tcpPort, int udpPort)
{
    tcpServer_.SetLogSink([](const std::string&) {});
    udpListener_.SetLogSink([](const std::string&) {});

    tcpServer_.Start(tcpPort);
    udpListener_.Open(udpPort);
}

void SimulatorApp::RunClientMode(const char* host, int tcpPort, int udpPort)
{
    const std::string target = (host != nullptr) ? host : "127.0.0.1";

    tcpClient_.SetLogSink([](const std::string&) {});
    udpBroadcaster_.SetLogSink([](const std::string&) {});

    tcpClient_.Connect(target, tcpPort);
    udpBroadcaster_.Open(target, udpPort);
}

} // namespace TcpUdpSimulator
