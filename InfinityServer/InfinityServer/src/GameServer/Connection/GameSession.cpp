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
    // 세션 레지스트리 해제 — 채팅 브로드캐스트 대상에서 제거
    if (m_roomId != 0)
        GameServer::Get().UnregisterSession(m_roomId, this);

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
                m_room          = room;
                m_roomId        = room->Id;
                m_authenticated = true;
                res.result      = 1;
                std::strncpy(res.message, "게임 접속 성공", sizeof(res.message) - 1);

                // 세션 레지스트리에 등록 — 채팅 브로드캐스트 대상이 됨
                GameServer::Get().RegisterSession(m_roomId, this);
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
            continue;
        }

        // ── 인게임 채팅 ───────────────────────────────
        //
        //  설계:
        //    클라이언트 → OP_GAME_CHAT_REQ → GameSession
        //    → GameServer::BroadcastToRoom → 룸 내 모든 GameSession::SendPacket
        //
        //  스레드 안전:
        //    BroadcastToRoom 내부에서 m_roomMutex 보유 중 세션 포인터만 복사 후 해제
        //    → LobbyRoom::Broadcast와 동일한 락 분리 전략
        //    각 GameSession::SendPacket은 m_sendMutex로 보호
        //
        //  Race Condition 대응:
        //    메시지 null-termination 강제
        //    미인증/룸 미배정 세션은 위 인증 차단에서 이미 걸러짐
        // ─────────────────────────────────────────────
        if (header.opcode == OP_GAME_CHAT_REQ)
        {
            if (header.body_size < sizeof(GameChatReqBody)) continue;
            if (m_roomId == 0) continue;  // 룸 미배정 방어

            const auto* req = reinterpret_cast<const GameChatReqBody*>(m_recvBuf.data());

            GameChatNtfyBody ntfy{};
            ntfy.server_tick = 0;  // ServerTick과 연동 시 현재 틱으로 교체
            ntfy.user_id     = m_userId;
            std::strncpy(ntfy.nickname, m_nickname, sizeof(ntfy.nickname) - 1);
            ntfy.nickname[sizeof(ntfy.nickname) - 1] = '\0';
            std::strncpy(ntfy.message, req->message, sizeof(ntfy.message) - 1);
            ntfy.message[sizeof(ntfy.message) - 1] = '\0';

            Logger::Write(LogLevel::Info, "game-chat",
                          "[룸" + std::to_string(m_roomId) + "] " +
                          std::string(ntfy.nickname) + ": " + ntfy.message);

            // BroadcastToRoom: 락 내 포인터 스냅샷 → 락 해제 → 송신
            GameServer::Get().BroadcastToRoom(m_roomId, OP_GAME_CHAT_NTFY,
                                              reinterpret_cast<const char*>(&ntfy),
                                              sizeof(ntfy));
            continue;
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
