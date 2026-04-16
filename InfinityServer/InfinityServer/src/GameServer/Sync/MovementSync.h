#pragma once

#include "GameServer/Packet/GamePacketDef.h"
#include <cstdint>
#include <vector>

class GameStateManager;
class GameSession;

// ─────────────────────────────────────────────────────
//  [게임 서버] 이동 동기화 — Authoritative Movement
//
//  처리 흐름:
//    1. 클라이언트가 OP_GAME_MOVE_INPUT (방향 + 틱번호) 전송
//    2. ProcessMoveInput(): 입력을 GameStateManager에 적용
//       - 방향 벡터를 정규화 후 최대 속도 제한 적용
//       - 위치는 서버 틱에서 TickPhysics()로 적분
//    3. BroadcastMove(): 갱신된 상태를 룸 전체에 브로드캐스트
//
//  치트 방지:
//    - 클라이언트가 위치를 직접 보내지 않음 (방향만 허용)
//    - 비정상 속도 요청 차단 (MAX_MOVE_SPEED 초과 시 무시)
//    - 죽은 플레이어 / 스턴 상태에서는 이동 입력 무시
//
//  지연 보상:
//    - 입력 패킷에 클라이언트 틱 번호 포함
//    - 서버가 틱 차이로 과거 상태 재현 가능 (LagCompensation 확장 가능)
// ─────────────────────────────────────────────────────
class MovementSync
{
public:
    explicit MovementSync(GameStateManager& stateManager);

    // 클라이언트 이동 입력 처리
    void ProcessMoveInput(int64_t userId, const GameMoveInputBody& input);

    // 갱신된 이동 상태를 세션 목록에 브로드캐스트
    void BroadcastMove(uint32_t serverTick,
                       int64_t  userId,
                       const std::vector<GameSession*>& sessions);

private:
    GameStateManager& m_stateManager;
};
