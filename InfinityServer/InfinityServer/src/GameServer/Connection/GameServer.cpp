#include "GameServer/Connection/GameServer.h"
#include "GameServer/Connection/GameSession.h"
#include "Shared/Logging/Logger.h"

#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

// GameRoom은 순환 참조 방지를 위해 전방 선언만 사용
// 실제 구현은 GameRoom.h/.cpp (GameServer/Tick/ 또는 별도 파일)
struct GameRoom
{
    uint32_t Id = 0;
};

GameServer* GameServer::s_instance = nullptr;

GameServer::GameServer(uint16_t port)
    : m_port(port)
    , m_listenSocket(INVALID_SOCKET)
    , m_running(false)
    , m_nextRoomId(1)
{
    s_instance = this;
}

GameServer::~GameServer()
{
    Stop();
    s_instance = nullptr;
}

GameServer& GameServer::Get()
{
    return *s_instance;
}

bool GameServer::Init()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        Logger::Write(LogLevel::Error, "game-server", "WSAStartup 실패");
        return false;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        Logger::Write(LogLevel::Error, "game-server", "소켓 생성 실패");
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
        Logger::Write(LogLevel::Error, "game-server", "bind/listen 실패");
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    Logger::Write(LogLevel::Info, "game-server",
                  "게임 서버 준비 완료 — 포트 " + std::to_string(m_port));
    return true;
}

void GameServer::Run()
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
                Logger::Write(LogLevel::Warning, "game-server", "accept 실패");
            continue;
        }

        GameSession* sess = new GameSession(sock, clientAddr);
        HANDLE h = CreateThread(nullptr, 0, ClientThread, sess, 0, nullptr);
        if (h) CloseHandle(h);
        else   { delete sess; }
    }
}

void GameServer::Stop()
{
    m_running = false;
    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        WSACleanup();
    }
}

DWORD WINAPI GameServer::ClientThread(LPVOID param)
{
    GameSession* sess = static_cast<GameSession*>(param);
    sess->Run();
    delete sess;
    return 0;
}

// ─────────────────────────────────────────────────────
//  match_token으로 룸을 찾거나 새로 생성
//  같은 token을 가진 플레이어들은 동일 룸에 배치됨
// ─────────────────────────────────────────────────────
GameRoom* GameServer::FindOrCreateRoom(const std::string& matchToken)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);

    auto it = m_tokenToRoomId.find(matchToken);
    if (it != m_tokenToRoomId.end())
    {
        auto roomIt = m_rooms.find(it->second);
        if (roomIt != m_rooms.end())
            return roomIt->second.get();
    }

    // 새 룸 생성
    uint32_t id = m_nextRoomId.fetch_add(1);
    auto room = std::make_unique<GameRoom>();
    room->Id = id;
    GameRoom* ptr = room.get();
    m_rooms[id]          = std::move(room);
    m_tokenToRoomId[matchToken] = id;

    Logger::Write(LogLevel::Info, "game-server",
                  "게임 룸 생성 id=" + std::to_string(id));
    return ptr;
}

GameRoom* GameServer::FindRoom(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);
    auto it = m_rooms.find(roomId);
    return (it != m_rooms.end()) ? it->second.get() : nullptr;
}

void GameServer::RemoveRoom(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);
    m_rooms.erase(roomId);
    Logger::Write(LogLevel::Info, "game-server",
                  "게임 룸 제거 id=" + std::to_string(roomId));
}
