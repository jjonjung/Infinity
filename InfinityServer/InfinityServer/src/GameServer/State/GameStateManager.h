#pragma once

#include "GameServer/State/PlayerState.h"
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

// ─────────────────────────────────────────────────────
//  [게임 서버] Authoritative 상태 관리자
//
//  설계:
//    - 룸 하나당 GameStateManager 인스턴스 하나
//    - 모든 PlayerState를 소유 (최대 4명)
//    - 서버 틱에서 Update() 호출 → 쿨다운 감소, 이동 적용
//
//  권위 서버(Authoritative Server) 원칙:
//    클라이언트는 입력만 전송하고 결과를 표시
//    서버가 입력을 받아 물리/히트 판정을 직접 계산
//    → 클라이언트가 위치를 조작해도 서버 상태에는 반영 안 됨
//
//  스레드 안전:
//    게임 서버 세션 스레드들이 동시에 상태를 갱신 가능
//    → m_mutex로 전체 상태 보호
// ─────────────────────────────────────────────────────
class GameStateManager
{
public:
    explicit GameStateManager(uint32_t roomId);

    // 플레이어 추가/제거
    bool          AddPlayer(const PlayerState& initial);
    void          RemovePlayer(int64_t userId);

    // 상태 접근 (뮤텍스 없이 — 호출자가 Lock 보유해야 함)
    PlayerState*  FindPlayer(int64_t userId);
    int           GetPlayerCount() const;

    // 서버 틱에서 호출 — 쿨다운/이동 등 시간 경과 처리
    void Update(uint32_t serverTick, float deltaSeconds);

    // 이동 입력 적용 (MovementSync에서 호출)
    void ApplyMoveInput(int64_t userId, float dirX, float dirY,
                        float yaw, bool jump, bool dash);

    // 히트 판정 적용 (서버 계산 결과)
    bool ApplyDamage(int64_t attackerId, int64_t victimId,
                     int32_t damage, uint8_t skillIndex);

    // 스냅샷/델타 수집 (StateSync에서 호출)
    std::vector<PlayerState> GetSnapshot() const;
    std::vector<PlayerState> DrainDirty();  // dirty 플레이어만 반환 후 dirty 클리어

    std::mutex& GetMutex() { return m_mutex; }

private:
    static constexpr float MAX_MOVE_SPEED = 600.f;  // 단위/초
    static constexpr float GRAVITY        = -980.f; // 단위/초²

    void TickCooldowns(float deltaSeconds);
    void TickPhysics(PlayerState& p, float deltaSeconds);

    uint32_t                 m_roomId;
    std::vector<PlayerState> m_players;  // 실제 데이터 — 캐시 지역성 유지
    // userId → m_players 인덱스: FindPlayer O(N) → O(1)
    // AddPlayer/RemovePlayer 시 동기화 유지 책임
    std::unordered_map<int64_t, size_t> m_playerIndex;
    mutable std::mutex       m_mutex;
};
