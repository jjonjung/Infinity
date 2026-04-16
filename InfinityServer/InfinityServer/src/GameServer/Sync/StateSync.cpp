#include "GameServer/Sync/StateSync.h"
#include "GameServer/State/GameStateManager.h"
#include "GameServer/Connection/GameSession.h"
#include "GameServer/Packet/GamePacketOpcodes.h"
#include "GameServer/Packet/GamePacketDef.h"

#include <algorithm>
#include <cstring>

StateSync::StateSync(GameStateManager& stateManager)
    : m_stateManager(stateManager)
{}

// ─────────────────────────────────────────────────────
//  전체 상태 스냅샷 전송
//
//  언제 사용:
//    - 플레이어 접속 직후 (초기 상태 수신)
//    - 5초마다 (패킷 손실 복구, 상태 재정렬)
//    - 서버가 상태를 강제 수정한 후 (버그 보정)
// ─────────────────────────────────────────────────────
void StateSync::SendSnapshot(uint32_t serverTick,
                             const std::vector<GameSession*>& sessions)
{
    auto players = m_stateManager.GetSnapshot();

    GameStateSnapshotBody body{};
    body.server_tick   = serverTick;
    body.player_count  = static_cast<uint8_t>(
                             std::min(players.size(), size_t(4)));

    for (uint8_t i = 0; i < body.player_count; ++i)
    {
        const PlayerState& p = players[i];
        PlayerSnapshotEntry& e = body.players[i];

        e.user_id = p.UserId;
        e.pos_x   = p.PosX;  e.pos_y = p.PosY;  e.pos_z = p.PosZ;
        e.vel_x   = p.VelX;  e.vel_y = p.VelY;  e.vel_z = p.VelZ;
        e.yaw     = p.Yaw;
        e.hp      = p.Hp;
        e.max_hp  = p.MaxHp;
        e.state   = p.StateFlags;
        std::memcpy(e.skill_cd, p.SkillCooldown, sizeof(e.skill_cd));
    }

    for (GameSession* sess : sessions)
        if (sess)
            sess->SendPacket(OP_GAME_STATE_SNAPSHOT,
                             reinterpret_cast<const char*>(&body), sizeof(body));
}

// ─────────────────────────────────────────────────────
//  델타 업데이트 전송
//
//  매 서버 틱(50ms)마다 호출
//  dirty_flags가 있는 플레이어만 포함 → 변경이 없으면 0바이트 전송 없음
//  DrainDirty()가 dirty 플래그를 초기화하므로 이중 전송 방지
// ─────────────────────────────────────────────────────
void StateSync::SendDelta(uint32_t serverTick,
                          const std::vector<GameSession*>& sessions)
{
    auto dirtyPlayers = m_stateManager.DrainDirty();
    if (dirtyPlayers.empty()) return;

    GameStateDeltaBody body{};
    body.server_tick  = serverTick;
    body.delta_count  = static_cast<uint8_t>(
                            std::min(dirtyPlayers.size(), size_t(4)));

    for (uint8_t i = 0; i < body.delta_count; ++i)
    {
        const PlayerState& p = dirtyPlayers[i];
        PlayerDeltaEntry& d  = body.deltas[i];

        d.user_id     = p.UserId;
        d.dirty_flags = p.DirtyFlags;

        if (p.DirtyFlags & DIRTY_HP)    d.hp    = p.Hp;
        if (p.DirtyFlags & DIRTY_STATE) d.state = p.StateFlags;
        if (p.DirtyFlags & DIRTY_POS)
        {
            d.pos_x = p.PosX;
            d.pos_y = p.PosY;
            d.pos_z = p.PosZ;
        }
    }

    for (GameSession* sess : sessions)
        if (sess)
            sess->SendPacket(OP_GAME_STATE_DELTA,
                             reinterpret_cast<const char*>(&body), sizeof(body));
}
