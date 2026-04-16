#include "GameServer/Sync/MovementSync.h"
#include "GameServer/State/GameStateManager.h"
#include "GameServer/State/PlayerState.h"
#include "GameServer/Connection/GameSession.h"
#include "GameServer/Packet/GamePacketOpcodes.h"
#include "Shared/Logging/Logger.h"

#include <cstring>
#include <string>

MovementSync::MovementSync(GameStateManager& stateManager)
    : m_stateManager(stateManager)
{}

// ─────────────────────────────────────────────────────
//  이동 입력 처리
//
//  클라이언트 tick을 기록해두면 이후 LagCompensation 구현 시
//  "클라이언트 시점의 상태"를 재현하는 데 활용 가능
// ─────────────────────────────────────────────────────
void MovementSync::ProcessMoveInput(int64_t userId, const GameMoveInputBody& input)
{
    m_stateManager.ApplyMoveInput(
        userId,
        input.dir_x,
        input.dir_y,
        input.yaw,
        input.jump != 0,
        input.dash != 0
    );
}

// ─────────────────────────────────────────────────────
//  이동 결과 브로드캐스트
//
//  서버 틱마다 호출 — dirty 플레이어의 위치/속도를 룸 전체에 전송
//  클라이언트는 이 값으로 Dead Reckoning 보정을 수행
// ─────────────────────────────────────────────────────
void MovementSync::BroadcastMove(uint32_t serverTick,
                                 int64_t  userId,
                                 const std::vector<GameSession*>& sessions)
{
    // StateManager에서 해당 플레이어 상태 조회
    auto snapshot = m_stateManager.GetSnapshot();
    for (const auto& p : snapshot)
    {
        if (p.UserId != userId) continue;
        if ((p.DirtyFlags & (DIRTY_POS | DIRTY_VEL)) == 0) break;

        GameMoveBroadcastBody body{};
        body.server_tick = serverTick;
        body.user_id     = p.UserId;
        body.pos_x       = p.PosX;
        body.pos_y       = p.PosY;
        body.pos_z       = p.PosZ;
        body.vel_x       = p.VelX;
        body.vel_y       = p.VelY;
        body.vel_z       = p.VelZ;
        body.yaw         = p.Yaw;

        for (GameSession* sess : sessions)
            if (sess)
                sess->SendPacket(OP_GAME_MOVE_BROADCAST,
                                 reinterpret_cast<const char*>(&body), sizeof(body));
        break;
    }
}
