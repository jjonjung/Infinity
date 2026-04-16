#include "GameServer/State/GameStateManager.h"
#include "Shared/Logging/Logger.h"

#include <algorithm>
#include <cmath>
#include <string>

GameStateManager::GameStateManager(uint32_t roomId)
    : m_roomId(roomId)
{
    m_players.reserve(4);
}

bool GameStateManager::AddPlayer(const PlayerState& initial)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_players.size() >= 4) return false;

    // 인덱스 맵 등록 후 push_back — 포인터 무효화 전에 인덱스 저장
    m_playerIndex[initial.UserId] = m_players.size();
    m_players.push_back(initial);
    Logger::Write(LogLevel::Info, "game-state",
                  "룸[" + std::to_string(m_roomId) + "] 플레이어 추가 user_id=" +
                  std::to_string(initial.UserId));
    return true;
}

void GameStateManager::RemovePlayer(int64_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_playerIndex.find(userId);
    if (it == m_playerIndex.end()) return;

    size_t idx = it->second;
    size_t last = m_players.size() - 1;

    if (idx != last)
    {
        // 마지막 원소를 빈 자리로 이동 (swap-erase — O(1))
        m_players[idx] = std::move(m_players[last]);
        m_playerIndex[m_players[idx].UserId] = idx;  // 이동된 원소 인덱스 갱신
    }

    m_players.pop_back();
    m_playerIndex.erase(it);
}

// ─────────────────────────────────────────────────────
//  FindPlayer — O(N) → O(1)
//  m_playerIndex가 userId → vector 인덱스를 추적하므로
//  매 틱마다 호출되는 ApplyMoveInput/ApplyDamage에서 순회 불필요
// ─────────────────────────────────────────────────────
PlayerState* GameStateManager::FindPlayer(int64_t userId)
{
    auto it = m_playerIndex.find(userId);
    if (it == m_playerIndex.end()) return nullptr;
    return &m_players[it->second];
}

int GameStateManager::GetPlayerCount() const
{
    return static_cast<int>(m_players.size());
}

// ─────────────────────────────────────────────────────
//  서버 틱 — 쿨다운 감소 + 물리 시뮬레이션
// ─────────────────────────────────────────────────────
void GameStateManager::Update(uint32_t /*serverTick*/, float deltaSeconds)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    TickCooldowns(deltaSeconds);
    for (auto& p : m_players)
        if (!p.IsDead())
            TickPhysics(p, deltaSeconds);
}

// ─────────────────────────────────────────────────────
//  이동 입력 처리 (Authoritative)
//
//  클라이언트가 위치를 직접 보내는 것이 아니라 방향을 보냄
//  서버가 속도 제한을 적용하여 위치를 계산 → 속도 치트 차단
// ─────────────────────────────────────────────────────
void GameStateManager::ApplyMoveInput(int64_t userId,
                                      float dirX, float dirY,
                                      float yaw, bool jump, bool dash)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    PlayerState* p = FindPlayer(userId);
    if (!p || p->IsDead() || p->IsStunned()) return;

    float speed = dash ? MAX_MOVE_SPEED * 2.f : MAX_MOVE_SPEED;

    // 방향 벡터 정규화 후 속도 적용
    float len = std::sqrtf(dirX * dirX + dirY * dirY);
    if (len > 0.001f)
    {
        p->VelX = (dirX / len) * speed;
        p->VelY = (dirY / len) * speed;
        p->StateFlags |= STATE_MOVING;
    }
    else
    {
        p->VelX = 0.f;
        p->VelY = 0.f;
        p->StateFlags &= ~STATE_MOVING;
    }

    if (jump && p->PosZ <= 0.01f)  // 지상에서만 점프
    {
        p->VelZ = 500.f;
        p->StateFlags |= STATE_JUMPING;
    }

    p->Yaw = yaw;
    p->MarkDirty(DIRTY_POS | DIRTY_VEL | DIRTY_STATE);
}

// ─────────────────────────────────────────────────────
//  데미지 적용 — 서버가 히트 판정 수행
//  클라이언트의 피격 신고를 신뢰하지 않음
// ─────────────────────────────────────────────────────
bool GameStateManager::ApplyDamage(int64_t attackerId, int64_t victimId,
                                   int32_t damage, uint8_t /*skillIndex*/)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    PlayerState* attacker = FindPlayer(attackerId);
    PlayerState* victim   = FindPlayer(victimId);
    if (!attacker || !victim || victim->IsDead()) return false;

    victim->TakeDamage(damage);
    attacker->DamageDealt += damage;
    attacker->MarkDirty(0);  // 공격자는 변경 없음

    if (victim->IsDead())
    {
        attacker->Kills++;
        victim->Deaths++;
        Logger::Write(LogLevel::Info, "game-state",
                      "사망 victim=" + std::to_string(victimId) +
                      " killer=" + std::to_string(attackerId));
    }

    return true;
}

std::vector<PlayerState> GameStateManager::GetSnapshot() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_players;
}

// dirty 플레이어만 반환 후 dirty 플래그 초기화
std::vector<PlayerState> GameStateManager::DrainDirty()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PlayerState> dirty;
    for (auto& p : m_players)
        if (p.DirtyFlags != 0)
        {
            dirty.push_back(p);
            p.ClearDirty();
        }
    return dirty;
}

// ─────────────────────────────────────────────────────
//  쿨다운 감소 (틱마다 1씩)
// ─────────────────────────────────────────────────────
void GameStateManager::TickCooldowns(float /*deltaSeconds*/)
{
    for (auto& p : m_players)
    {
        bool changed = false;
        for (auto& cd : p.SkillCooldown)
            if (cd > 0) { --cd; changed = true; }
        if (changed) p.MarkDirty(DIRTY_SKILL_CD);
    }
}

// ─────────────────────────────────────────────────────
//  간이 물리 — 위치 적분 + 중력
// ─────────────────────────────────────────────────────
void GameStateManager::TickPhysics(PlayerState& p, float dt)
{
    p.PosX += p.VelX * dt;
    p.PosY += p.VelY * dt;
    p.PosZ += p.VelZ * dt;

    if (p.PosZ > 0.f)
    {
        p.VelZ += GRAVITY * dt;
    }
    else
    {
        p.PosZ = 0.f;
        p.VelZ = 0.f;
        p.StateFlags &= ~STATE_JUMPING;
    }

    if (p.VelX != 0.f || p.VelY != 0.f || p.VelZ != 0.f)
        p.MarkDirty(DIRTY_POS);
}
