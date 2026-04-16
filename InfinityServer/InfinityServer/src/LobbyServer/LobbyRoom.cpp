#include "LobbyServer/LobbyRoom.h"
#include "LobbyServer/LobbyPacketDef.h"
#include "LobbyServer/LobbySession.h"
#include "Shared/Logging/Logger.h"

#include <chrono>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>

LobbyRoom::LobbyRoom(uint32_t id, const std::string& name, uint8_t maxPlayers)
    : m_id(id)
    , m_maxPlayers(maxPlayers > MAX_PLAYERS ? MAX_PLAYERS : maxPlayers)
    , m_started(false)
{
    std::strncpy(m_name, name.c_str(), sizeof(m_name) - 1);
    m_name[sizeof(m_name) - 1] = '\0';
    m_slots = {};  // 전체 슬롯 zero-init
}

bool LobbyRoom::AddPlayer(LobbySession* session)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_started || m_playerCount >= m_maxPlayers) return false;

    m_slots[m_playerCount++] = { session, false };
    Logger::Write(LogLevel::Info, "lobby-room",
                  "방[" + std::to_string(m_id) + "] 입장 user_id=" +
                  std::to_string(session->GetUserId()) +
                  " (" + std::to_string(m_playerCount) + "/" +
                  std::to_string(m_maxPlayers) + ")");
    return true;
}

// 제거 후 뒤 원소들을 앞으로 당겨 배열 연속성 유지
void LobbyRoom::RemovePlayer(int64_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (uint8_t i = 0; i < m_playerCount; ++i)
    {
        if (m_slots[i].Session && m_slots[i].Session->GetUserId() == userId)
        {
            for (uint8_t j = i; j < m_playerCount - 1; ++j)
                m_slots[j] = m_slots[j + 1];
            m_slots[--m_playerCount] = {};  // 마지막 슬롯 클리어
            break;
        }
    }
}

void LobbyRoom::SetReady(int64_t userId, bool ready)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (uint8_t i = 0; i < m_playerCount; ++i)
        if (m_slots[i].Session && m_slots[i].Session->GetUserId() == userId)
        {
            m_slots[i].IsReady = ready;
            break;
        }
}

bool LobbyRoom::IsFull() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playerCount >= m_maxPlayers;
}

bool LobbyRoom::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playerCount == 0;
}

bool LobbyRoom::AllReady() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playerCount < m_maxPlayers) return false;
    for (uint8_t i = 0; i < m_playerCount; ++i)
        if (!m_slots[i].IsReady) return false;
    return true;
}

uint8_t LobbyRoom::GetPlayerCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playerCount;
}

void LobbyRoom::StartMatch(const char* gameServerIp, uint16_t gameServerPort)
{
    if (m_started.exchange(true)) return;

    MatchStartNtfyBody body{};
    std::strncpy(body.game_server_ip, gameServerIp, sizeof(body.game_server_ip) - 1);
    body.game_server_port = gameServerPort;
    std::string token = GenerateMatchToken();
    std::strncpy(body.match_token, token.c_str(), sizeof(body.match_token) - 1);

    Logger::Write(LogLevel::Info, "lobby-room",
                  "방[" + std::to_string(m_id) + "] 매치 시작 → " +
                  std::string(gameServerIp) + ":" + std::to_string(gameServerPort));

    Broadcast(OP_MATCH_START_NTFY,
              reinterpret_cast<const char*>(&body), sizeof(body));
}

// ─────────────────────────────────────────────────────
//  [병목 수정] Broadcast — 락 분리 전송
//
//  이전: m_mutex 보유 중 send() 호출
//    → TCP 버퍼 포화 또는 느린 클라이언트로 send() 블로킹
//    → 다른 스레드의 AddPlayer/RemovePlayer 전체 차단
//
//  이후: 락 내부에서 세션 포인터만 스냅샷 복사 → 락 해제 → 송신
//    → send() 블로킹이 룸 상태 조작과 완전히 분리됨
// ─────────────────────────────────────────────────────
void LobbyRoom::Broadcast(uint16_t opcode, const char* body, uint16_t bodySize)
{
    // 1단계: 락 보유 시간을 포인터 복사로만 최소화
    LobbySession* snapshot[MAX_PLAYERS] = {};
    uint8_t count = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (uint8_t i = 0; i < m_playerCount; ++i)
            if (m_slots[i].Session)
                snapshot[count++] = m_slots[i].Session;
    }

    // 2단계: 락 해제 후 송신 — 블로킹이 룸 조작에 영향 없음
    for (uint8_t i = 0; i < count; ++i)
        snapshot[i]->SendPacket(opcode, body, bodySize);
}

// ─────────────────────────────────────────────────────
//  [메모리 수정] thread_local rng — 매 호출마다 재생성 제거
//
//  이전: 호출할 때마다 mt19937_64 객체 생성 + 시드 연산
//  이후: 스레드당 1회 초기화, 이후 호출은 next() 만 수행
// ─────────────────────────────────────────────────────
std::string LobbyRoom::GenerateMatchToken() const
{
    thread_local std::mt19937_64 s_rng(
        static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        ^ reinterpret_cast<uint64_t>(&s_rng));  // 스레드 주소로 시드 추가 분산

    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << dist(s_rng)
        << std::setw(16) << dist(s_rng)
        << std::setw(16) << dist(s_rng)
        << std::setw(16) << dist(s_rng);
    return oss.str();
}
