#include "GameServer/Connection/GameSession.h"
#include "GameServer/Connection/GameServer.h"
#include "GameServer/Packet/GamePacketOpcodes.h"
#include "GameServer/Packet/GamePacketDef.h"
#include "Packet/Packet.h"
#include "Bootstrap/ServerRuntime.h"
#include "Shared/Logging/Logger.h"

#include <cstring>
#include <string>

// 게임 패킷은 이동 입력이 많아 버퍼를 충분히 확보
static constexpr uint16_t MAX_GAME_BODY = 4096;

std::atomic<int> GameSession::s_count = 0;

GameSession::GameSession(SOCKET socket, sockaddr_in addr)
    : m_socket(socket)
    , m_addr(addr)
{
    inet_ntop(AF_INET, &m_addr.sin_addr, m_clientIP, sizeof(m_clientIP));
    m_recvBuf.reserve(MAX_GAME_BODY);
    m_sendBuf.reserve(HEADER_SIZE + MAX_GAME_BODY);
    ++s_count;
}

GameSession::~GameSession()
{
    if (m_room && m_userId)
    {
        // 룸에서 자신을 제거 (구체적인 처리는 GameRoom에서)
        // GameRoom::RemoveSession(m_userId) 호출은 GameRoom.cpp에서 처리
    }

    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    --s_count;
}

// ─────────────────────────────────────────────────────
//  게임 세션 수신 루프
//  접속 → match_token 검증 → 게임 패킷 처리
// ─────────────────────────────────────────────────────
void GameSession::Run()
{
    PacketHeader header{};
    while (true)
    {
        if (!RecvAll(reinterpret_cast<char*>(&header), static_cast<int>(HEADER_SIZE)))
        {
            Logger::Write(LogLevel::Info, "game-session",
                          std::string(m_clientIP) + " 연결 종료");
            break;
        }

        if (header.body_size > MAX_GAME_BODY)
        {
            Logger::Write(LogLevel::Warning, "game-session",
                          std::string(m_clientIP) + " 비정상 패킷 크기 차단");
            break;
        }

        m_recvBuf.resize(header.body_size);
        if (header.body_size > 0 &&
            !RecvAll(m_recvBuf.data(), static_cast<int>(header.body_size)))
            break;

        // ── 접속 요청: 인증 전 처리 ──────────────────
        if (header.opcode == OP_GAME_CONNECT_REQ)
        {
            if (header.body_size < sizeof(GameConnectReqBody)) break;
            const auto* req = reinterpret_cast<const GameConnectReqBody*>(m_recvBuf.data());

            GameRoom* room = GameServer::Get().FindOrCreateRoom(req->match_token);
            GameConnectResBody res{};
            if (room)
            {
                m_room = room;
                // GameRoom::AddSession은 GameRoom.cpp 구현에서 user_id 설정
                m_authenticated = true;
                res.result = 1;
                std::strncpy(res.message, "게임 접속 성공", sizeof(res.message) - 1);
            }
            else
            {
                res.result = 0;
                std::strncpy(res.message, "match_token 검증 실패", sizeof(res.message) - 1);
                SendPacket(OP_GAME_CONNECT_RES,
                           reinterpret_cast<const char*>(&res), sizeof(res));
                break;
            }
            SendPacket(OP_GAME_CONNECT_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
            continue;
        }

        // ── 인증 전 다른 패킷 차단 ───────────────────
        if (!m_authenticated)
        {
            Logger::Write(LogLevel::Warning, "game-session",
                          "미인증 opcode: " + std::to_string(header.opcode));
            break;
        }

        // ── 게임 패킷 라우팅 ─────────────────────────
        // 이동/스킬/핑은 GameRoom을 통해 MovementSync/StateSync로 전달
        // (GameRoom::DispatchPacket 구현 참조)
        if (m_room)
        {
            // GameRoom::DispatchPacket(*this, header.opcode, m_recvBuf.data(), header.body_size);
        }

        // 핑 응답은 세션에서 직접 처리 (룸 거치면 지연 증가)
        if (header.opcode == OP_GAME_PING)
        {
            if (header.body_size >= sizeof(GamePingBody))
            {
                const auto* ping = reinterpret_cast<const GamePingBody*>(m_recvBuf.data());
                GamePongBody pong{ ping->client_time_ms, 0 /* server time */ };
                SendPacket(OP_GAME_PONG,
                           reinterpret_cast<const char*>(&pong), sizeof(pong));
            }
        }
    }
}

void GameSession::SendPacket(uint16_t opcode, const char* body, uint16_t bodySize)
{
    PacketHeader hdr;
    hdr.body_size = bodySize;
    hdr.opcode    = opcode;

    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_sendBuf.resize(HEADER_SIZE + bodySize);
    std::memcpy(m_sendBuf.data(), &hdr, HEADER_SIZE);
    if (bodySize > 0 && body)
        std::memcpy(m_sendBuf.data() + HEADER_SIZE, body, bodySize);

    send(m_socket, m_sendBuf.data(), static_cast<int>(m_sendBuf.size()), 0);
}

bool GameSession::RecvAll(char* buf, int size)
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

int GameSession::GetActiveCount()
{
    return s_count.load();
}
