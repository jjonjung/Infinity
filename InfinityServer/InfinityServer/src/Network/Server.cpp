#include "Network/Server.h"
#include "Network/Session.h"

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

Server::Server(uint16_t port)
    : m_port(port)
    , m_listenSocket(INVALID_SOCKET)
    , m_running(false)
{}

Server::~Server()
{
    Stop();
}

// ─────────────────────────────────────────────────────
//  Winsock 초기화 및 리슨 소켓 바인딩
// ─────────────────────────────────────────────────────
bool Server::Init()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "[서버] WSAStartup 실패 — " << WSAGetLastError() << "\n";
        return false;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        std::cerr << "[서버] 소켓 생성 실패 — " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

    // 빠른 재시작을 위해 SO_REUSEADDR 설정
    int optval = 1;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<char*>(&optval), sizeof(optval));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(m_port);

    if (bind(m_listenSocket,
             reinterpret_cast<SOCKADDR*>(&addr),
             sizeof(addr)) == SOCKET_ERROR)
    {
        std::cerr << "[서버] bind 실패 — " << WSAGetLastError() << "\n";
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[서버] listen 실패 — " << WSAGetLastError() << "\n";
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    return true;
}

// ─────────────────────────────────────────────────────
//  accept 루프 — 클라이언트마다 스레드 1개 생성
// ─────────────────────────────────────────────────────
void Server::Run()
{
    m_running = true;
    std::cout << "[서버] 시작 — 포트 " << m_port << " 대기 중...\n";

    while (m_running)
    {
        sockaddr_in clientAddr{};
        int addrLen = static_cast<int>(sizeof(clientAddr));

        SOCKET clientSocket = accept(
            m_listenSocket,
            reinterpret_cast<SOCKADDR*>(&clientAddr),
            &addrLen
        );

        if (clientSocket == INVALID_SOCKET)
        {
            if (m_running)
                std::cerr << "[서버] accept 실패 — " << WSAGetLastError() << "\n";
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
        std::cout << "[서버] 클라이언트 접속: "
                  << ip << ":" << ntohs(clientAddr.sin_port) << "\n";

        Session* session = new Session(clientSocket, clientAddr);
        HANDLE hThread   = CreateThread(nullptr, 0, ClientThread, session, 0, nullptr);

        if (hThread)
        {
            CloseHandle(hThread);  // 스레드 핸들 즉시 반환 (detach)
        }
        else
        {
            std::cerr << "[서버] 스레드 생성 실패 — " << GetLastError() << "\n";
            delete session;
        }
    }
}

void Server::Stop()
{
    m_running = false;
    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        WSACleanup();
    }
}

// ─────────────────────────────────────────────────────
//  클라이언트 스레드 진입점
// ─────────────────────────────────────────────────────
DWORD WINAPI Server::ClientThread(LPVOID param)
{
    Session* session = static_cast<Session*>(param);
    session->Run();
    delete session;
    return 0;
}
