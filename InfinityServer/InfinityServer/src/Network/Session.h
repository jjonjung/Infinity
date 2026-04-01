#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>

class Session
{
public:
    Session(SOCKET socket, sockaddr_in addr);
    ~Session();

    // 메인 루프 — 클라이언트 스레드에서 호출
    void Run();

    // PacketHandler에서 호출
    void SendPacket(uint16_t opcode, const char* body, uint16_t bodySize);

private:
    bool RecvAll(char* buf, int size);

    SOCKET      m_socket;
    sockaddr_in m_addr;
    char        m_clientIP[INET_ADDRSTRLEN];
};
