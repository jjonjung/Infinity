#pragma once

#include "GameServer/State/PlayerState.h"
#include <cstdint>
#include <vector>

class GameStateManager;
class GameSession;

// ─────────────────────────────────────────────────────
//  [게임 서버] 상태 동기화 — Authoritative State Sync
//
//  두 가지 전송 방식:
//
//  1. 스냅샷 (Full Snapshot)
//     - 모든 플레이어의 전체 상태를 전송
//     - 접속 직후 or 5초마다 전송 (연결 복구용)
//     - 대역폭 높지만 누락 패킷을 자동 복구
//
//  2. 델타 업데이트 (Delta Update)
//     - dirty_flags가 설정된 플레이어만 변경된 필드 전송
//     - 매 서버 틱(50ms)마다 전송
//     - 스냅샷 대비 대역폭 절약 (HP 변경 시 위치는 생략 등)
//
//  클라이언트 처리:
//    - 스냅샷: 받은 즉시 상태를 덮어씀
//    - 델타: 기존 상태에 변경 필드만 적용 + Dead Reckoning으로 위치 보간
// ─────────────────────────────────────────────────────
class StateSync
{
public:
    explicit StateSync(GameStateManager& stateManager);

    // 전체 상태 스냅샷 전송 — 접속 직후 / 주기적 리셋
    void SendSnapshot(uint32_t serverTick,
                      const std::vector<GameSession*>& sessions);

    // dirty 플레이어만 델타 전송 — 매 서버 틱 호출
    void SendDelta(uint32_t serverTick,
                   const std::vector<GameSession*>& sessions);

private:
    GameStateManager& m_stateManager;
};
