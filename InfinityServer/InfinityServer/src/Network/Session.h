#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <cstdint>
#include <vector>

class Session
{
public:
    Session(SOCKET socket, sockaddr_in addr);
    ~Session();

    // 메인 루프 — 클라이언트 스레드에서 호출
    void Run();

    // PacketHandler에서 호출
    void SendPacket(uint16_t opcode, const char* body, uint16_t bodySize);
    static int GetActiveSessionCount();

private:
    bool RecvAll(char* buf, int size);

    SOCKET      m_socket;
    sockaddr_in m_addr;
    char        m_clientIP[INET_ADDRSTRLEN];
    std::vector<char> m_receiveBuffer;
    std::vector<char> m_sendBuffer;
    static std::atomic<int> s_activeSessions;
};
