#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Packet/Packet.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <string>
#include <vector>

struct PacketEnvelope
{
    uint16_t Opcode = 0;
    std::vector<char> Body;
};

class TestClientConnection
{
public:
    TestClientConnection();
    ~TestClientConnection();

    bool Connect(const std::string& host, uint16_t port, std::string& errorMessage);
    void Disconnect();
    bool IsConnected() const;

    bool SendPacket(uint16_t opcode, const void* body, uint16_t bodySize, std::string& errorMessage);
    bool ReceivePacket(PacketEnvelope& packet, std::string& errorMessage);

private:
    bool RecvAll(char* buffer, int size, std::string& errorMessage);

    SOCKET m_socket;
    bool m_wsaStarted;
};
