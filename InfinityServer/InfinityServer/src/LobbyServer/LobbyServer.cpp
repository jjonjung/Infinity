#include "LobbyServer/LobbyServer.h"
#include "LobbyServer/LobbyRoom.h"
#include "LobbyServer/LobbySession.h"
#include "Shared/Logging/Logger.h"

#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

LobbyServer* LobbyServer::s_instance = nullptr;

LobbyServer::LobbyServer(uint16_t port)
    : m_port(port)
    , m_listenSocket(INVALID_SOCKET)
    , m_running(false)
    , m_nextRoomId(1)
{
    s_instance = this;
}

LobbyServer::~LobbyServer()
{
    Stop();
    s_instance = nullptr;
}

LobbyServer& LobbyServer::Get()
{
    return *s_instance;
}

bool LobbyServer::Init()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        Logger::Write(LogLevel::Error, "lobby-server", "WSAStartup 실패");
        return false;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        Logger::Write(LogLevel::Error, "lobby-server", "소켓 생성 실패");
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
        Logger::Write(LogLevel::Error, "lobby-server", "bind/listen 실패");
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    Logger::Write(LogLevel::Info, "lobby-server",
                  "로비 서버 준비 완료 — 포트 " + std::to_string(m_port));
    return true;
}

void LobbyServer::Run()
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
                Logger::Write(LogLevel::Warning, "lobby-server", "accept 실패");
            continue;
        }

        LobbySession* sess = new LobbySession(sock, clientAddr);
        HANDLE h = CreateThread(nullptr, 0, ClientThread, sess, 0, nullptr);
        if (h) CloseHandle(h);
        else   { delete sess; }
    }
}

void LobbyServer::Stop()
{
    m_running = false;
    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        WSACleanup();
    }
}

DWORD WINAPI LobbyServer::ClientThread(LPVOID param)
{
    LobbySession* sess = static_cast<LobbySession*>(param);
    sess->Run();
    delete sess;
    return 0;
}

// ─────────────────────────────────────────────────────
//  방 관리 (LobbyPacketHandler에서 호출)
// ─────────────────────────────────────────────────────
uint32_t LobbyServer::CreateRoom(const std::string& name, uint8_t maxPlayers)
{
    uint32_t id = m_nextRoomId.fetch_add(1);
    auto room = std::make_unique<LobbyRoom>(id, name, maxPlayers);

    std::lock_guard<std::mutex> lock(m_roomMutex);
    m_rooms[id] = std::move(room);
    Logger::Write(LogLevel::Info, "lobby-server",
                  "방 생성 id=" + std::to_string(id) + " name=" + name +
                  " max=" + std::to_string(maxPlayers));
    return id;
}

LobbyRoom* LobbyServer::FindRoom(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);
    auto it = m_rooms.find(roomId);
    return (it != m_rooms.end()) ? it->second.get() : nullptr;
}

void LobbyServer::RemoveRoomIfEmpty(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);
    auto it = m_rooms.find(roomId);
    if (it != m_rooms.end() && it->second->IsEmpty())
    {
        m_rooms.erase(it);
        Logger::Write(LogLevel::Info, "lobby-server",
                      "빈 방 제거 id=" + std::to_string(roomId));
    }
}

std::vector<LobbyRoom*> LobbyServer::GetAllRooms()
{
    std::lock_guard<std::mutex> lock(m_roomMutex);
    std::vector<LobbyRoom*> list;
    list.reserve(m_rooms.size());
    for (auto& kv : m_rooms)
        list.push_back(kv.second.get());
    return list;
}
