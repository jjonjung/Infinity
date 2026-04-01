#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <cstdint>

class Server
{
public:
    explicit Server(uint16_t port);
    ~Server();

    bool Init();
    void Run();   // accept 루프 (blocking)
    void Stop();

private:
    static DWORD WINAPI ClientThread(LPVOID param);  // 클라이언트당 1 스레드

    uint16_t m_port;
    SOCKET   m_listenSocket;
    bool     m_running;
};
