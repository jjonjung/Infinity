#include "LoginServer/LoginServer.h"
#include "LoginServer/LoginSession.h"
#include "Shared/Logging/Logger.h"

#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

LoginServer::LoginServer(uint16_t port)
    : m_port(port)
    , m_listenSocket(INVALID_SOCKET)
    , m_running(false)
{}

LoginServer::~LoginServer()
{
    Stop();
}

bool LoginServer::Init()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        Logger::Write(LogLevel::Error, "login-server", "WSAStartup 실패");
        return false;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        Logger::Write(LogLevel::Error, "login-server", "소켓 생성 실패");
        WSACleanup();
        return false;
    }

    int opt = 1;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(m_port);

    if (bind(m_listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR ||
        listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        Logger::Write(LogLevel::Error, "login-server", "bind/listen 실패");
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    Logger::Write(LogLevel::Info, "login-server",
                  "인증 서버 준비 완료 — 포트 " + std::to_string(m_port));
    return true;
}

// ─────────────────────────────────────────────────────
//  accept 루프 — 접속마다 스레드 1개 생성
//  로그인 서버는 동시 접속 수가 적어 스레드-per-클라이언트가 적합
// ─────────────────────────────────────────────────────
void LoginServer::Run()
{
    m_running = true;
    while (m_running)
    {
        sockaddr_in clientAddr{};
        int len = static_cast<int>(sizeof(clientAddr));

        SOCKET sock = accept(m_listenSocket,
                             reinterpret_cast<SOCKADDR*>(&clientAddr), &len);
        if (sock == INVALID_SOCKET)
        {
            if (m_running)
                Logger::Write(LogLevel::Warning, "login-server", "accept 실패");
            continue;
        }

        LoginSession* sess = new LoginSession(sock, clientAddr);
        HANDLE h = CreateThread(nullptr, 0, ClientThread, sess, 0, nullptr);
        if (h)
            CloseHandle(h);
        else
        {
            Logger::Write(LogLevel::Error, "login-server", "스레드 생성 실패");
            delete sess;
        }
    }
}

void LoginServer::Stop()
{
    m_running = false;
    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        WSACleanup();
    }
}

DWORD WINAPI LoginServer::ClientThread(LPVOID param)
{
    LoginSession* sess = static_cast<LoginSession*>(param);
    sess->Run();
    delete sess;
    return 0;
}
