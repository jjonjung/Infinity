#pragma once

#include "GameServer/Packet/GamePacketDef.h"
#include <cstdint>
#include <string>

// ─────────────────────────────────────────────────────
//  [게임 서버] 상태 관리 — PlayerState
//
//  Authoritative 서버의 핵심: 서버가 모든 플레이어의
//  "진짜" 상태를 소유하고 클라이언트는 복사본만 가짐
//
//  갱신 원칙:
//    - 위치/속도: 서버 틱마다 MoveInput으로 업데이트
//    - HP: 서버 히트 판정 시에만 감소
//    - 스킬 쿨다운: 서버 틱마다 자동 감소
//    - 상태 플래그: 각 액션 시작/종료 시 변경
//
//  클라이언트 동기화:
//    - 변경된 필드 추적용 dirty_flags 유지
//    - StateSync가 dirty_flags 확인 후 델타 또는 스냅샷 전송
// ─────────────────────────────────────────────────────

struct PlayerState
{
    int64_t     UserId      = 0;
    char        Nickname[32]= {};
    char        Character[32]={};
    uint8_t     SlotIndex   = 0;        // 룸 내 슬롯 (0~3)

    // 위치/이동
    float       PosX = 0.f, PosY = 0.f, PosZ = 0.f;
    float       VelX = 0.f, VelY = 0.f, VelZ = 0.f;
    float       Yaw  = 0.f;

    // 전투
    int32_t     Hp    = 100;
    int32_t     MaxHp = 100;
    int32_t     Kills = 0, Deaths = 0, Assists = 0;
    int32_t     DamageDealt = 0;

    // 스킬 쿨다운 (틱 단위, 0이면 사용 가능)
    uint16_t    SkillCooldown[4] = {};

    // 상태 플래그 (PlayerStateFlag 비트 조합)
    uint8_t     StateFlags = STATE_IDLE;

    // 더티 추적 (StateSync에서 읽고 자동 클리어)
    uint16_t    DirtyFlags = 0;

    // 편의 메서드
    bool IsDead()    const { return (StateFlags & STATE_DEAD) != 0; }
    bool IsStunned() const { return (StateFlags & STATE_STUNNED) != 0; }

    void MarkDirty(uint16_t flags) { DirtyFlags |= flags; }
    void ClearDirty()              { DirtyFlags = 0; }

    void TakeDamage(int32_t dmg)
    {
        Hp -= dmg;
        if (Hp <= 0) { Hp = 0; StateFlags |= STATE_DEAD; }
        MarkDirty(DIRTY_HP | DIRTY_STATE);
    }

    void Respawn(float x, float y, float z)
    {
        Hp     = MaxHp;
        PosX = x; PosY = y; PosZ = z;
        VelX = VelY = VelZ = 0.f;
        StateFlags = STATE_IDLE;
        MarkDirty(DIRTY_HP | DIRTY_POS | DIRTY_STATE);
    }
};
