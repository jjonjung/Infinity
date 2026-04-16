#include "LobbyServer/LobbySession.h"
#include "LobbyServer/LobbyPacketHandler.h"
#include "LobbyServer/LobbyRoom.h"
#include "LobbyServer/LobbyServer.h"
#include "Packet/Packet.h"
#include "Shared/Logging/Logger.h"

#include <cstring>
#include <string>

static constexpr uint16_t MAX_LOBBY_BODY = 2048;

std::atomic<int> LobbySession::s_count = 0;

LobbySession::LobbySession(SOCKET socket, sockaddr_in addr)
    : m_socket(socket)
    , m_addr(addr)
{
    inet_ntop(AF_INET, &m_addr.sin_addr, m_clientIP, sizeof(m_clientIP));
    m_recvBuf.reserve(MAX_LOBBY_BODY);
    m_sendBuf.reserve(HEADER_SIZE + MAX_LOBBY_BODY);
    ++s_count;
}

LobbySession::~LobbySession()
{
    // 방에 참가 중인 상태로 연결이 끊기면 자동 퇴장 처리
    if (m_room)
    {
        m_room->RemovePlayer(m_userId);
        LobbyServer::Get().RemoveRoomIfEmpty(m_room->GetId());
        m_room = nullptr;
    }

    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    --s_count;
}

// ─────────────────────────────────────────────────────
//  로비 세션은 연결을 유지 — 방에 있는 동안 패킷 루프 지속
//  로그인 서버(단일 요청-응답)와 달리 stateful 처리
// ─────────────────────────────────────────────────────
void LobbySession::Run()
{
    PacketHeader header{};
    while (true)
    {
        if (!RecvAll(reinterpret_cast<char*>(&header), static_cast<int>(HEADER_SIZE)))
        {
            Logger::Write(LogLevel::Info, "lobby-session",
                          std::string(m_clientIP) + " 연결 종료");
            break;
        }

        if (header.body_size > MAX_LOBBY_BODY)
        {
            Logger::Write(LogLevel::Warning, "lobby-session",
                          std::string(m_clientIP) + " 비정상 패킷 크기 차단");
            break;
        }

        m_recvBuf.resize(header.body_size);
        if (header.body_size > 0 &&
            !RecvAll(m_recvBuf.data(), static_cast<int>(header.body_size)))
            break;

        LobbyPacketHandler::Handle(*this, header.opcode,
                                   m_recvBuf.data(), header.body_size);
    }
}

void LobbySession::SendPacket(uint16_t opcode, const char* body, uint16_t bodySize)
{
    PacketHeader hdr;
    hdr.body_size = bodySize;
    hdr.opcode    = opcode;

    // Broadcast는 다른 세션의 스레드에서 호출될 수 있으므로 뮤텍스 필요
    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_sendBuf.resize(HEADER_SIZE + bodySize);
    std::memcpy(m_sendBuf.data(), &hdr, HEADER_SIZE);
    if (bodySize > 0 && body)
        std::memcpy(m_sendBuf.data() + HEADER_SIZE, body, bodySize);

    send(m_socket, m_sendBuf.data(), static_cast<int>(m_sendBuf.size()), 0);
}

void LobbySession::SetAuthenticated(int64_t userId, const std::string& nickname)
{
    m_authenticated = true;
    m_userId = userId;
    std::strncpy(m_nickname, nickname.c_str(), sizeof(m_nickname) - 1);
}

void LobbySession::SetRoom(LobbyRoom* room)
{
    m_room = room;
}

void LobbySession::ClearRoom()
{
    m_room = nullptr;
}

bool LobbySession::RecvAll(char* buf, int size)
{
    int recvd = 0;
    while (recvd < size)
    {
        int r = recv(m_socket, buf + recvd, size - recvd, 0);
        if (r <= 0) return false;
        recvd += r;
    }
    return true;
}

int LobbySession::GetActiveCount()
{
    return s_count.load();
}
