#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

class LobbySession;

// ─────────────────────────────────────────────────────
//  [로비 서버] 방(Room) — 매치 준비 단위
//
//  플레이어 구성:
//    - 최대 MAX_PLAYERS 명 (4 고정)
//    - 방장(host)은 첫 번째 입장자
//
//  게임 시작 조건:
//    - 인원 == m_maxPlayers AND 전원 IsReady == true
//    - StartMatch() 호출 → 게임 서버 주소 + match_token 브로드캐스트
//
//  스레드 안전:
//    모든 멤버 접근은 m_mutex로 보호
//    Broadcast()는 락 내부에서 세션 목록을 복사한 뒤 락을 해제하고 송신
//    → send() 블로킹이 룸 상태 조작을 막지 않음
//
//  메모리:
//    std::array<PlayerSlot, MAX_PLAYERS> — 힙 할당 없이 스택/인라인 배치
// ─────────────────────────────────────────────────────
class LobbyRoom
{
public:
    static constexpr uint8_t MAX_PLAYERS = 4;

    LobbyRoom(uint32_t id, const std::string& name, uint8_t maxPlayers);

    bool AddPlayer    (LobbySession* session);
    void RemovePlayer (int64_t userId);
    void SetReady     (int64_t userId, bool ready);

    bool IsFull()   const;
    bool IsEmpty()  const;
    bool AllReady() const;

    uint32_t    GetId()          const { return m_id; }
    const char* GetName()        const { return m_name; }
    uint8_t     GetMaxPlayers()  const { return m_maxPlayers; }
    uint8_t     GetPlayerCount() const;
    bool        IsStarted()      const { return m_started.load(); }

    // 전원 준비 시 게임 서버 주소와 match_token을 방 안 전원에게 브로드캐스트
    void StartMatch(const char* gameServerIp, uint16_t gameServerPort);

    // 락 해제 후 송신 — send() 블로킹이 룸 조작을 차단하지 않음
    void Broadcast(uint16_t opcode, const char* body, uint16_t bodySize);

private:
    struct PlayerSlot
    {
        LobbySession* Session = nullptr;
        bool          IsReady = false;
    };

    std::string GenerateMatchToken() const;

    uint32_t          m_id;
    char              m_name[32];
    uint8_t           m_maxPlayers;
    uint8_t           m_playerCount = 0;   // 유효 슬롯 수
    std::atomic<bool> m_started;

    mutable std::mutex                  m_mutex;
    std::array<PlayerSlot, MAX_PLAYERS> m_slots; // 고정 크기 — 힙 불필요
};
