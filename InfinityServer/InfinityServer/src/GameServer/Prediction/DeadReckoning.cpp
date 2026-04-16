#include "GameServer/Prediction/DeadReckoning.h"

#include <cmath>
#include <cstring>

DeadReckoning::DeadReckoning()
{
    std::memset(m_history, 0, sizeof(m_history));
}

// ─────────────────────────────────────────────────────
//  스냅샷 추가 — 링 버퍼에 최신 권위 상태 저장
// ─────────────────────────────────────────────────────
void DeadReckoning::AddSnapshot(const PositionSnapshot& snap)
{
    m_head = (m_head + 1) % HISTORY_SIZE;
    m_history[m_head] = snap;
    if (m_count < HISTORY_SIZE) ++m_count;
}

// ─────────────────────────────────────────────────────
//  위치 예측 (Dead Reckoning)
//
//  마지막 권위 위치에서 속도 × 경과 시간으로 현재 위치 추정
//  실제 물리(중력, 충돌)는 서버 권위이므로 예측은 근사치
// ─────────────────────────────────────────────────────
PredictedPosition DeadReckoning::Predict(uint32_t currentTick,
                                         float deltaSeconds) const
{
    const PositionSnapshot* latest = LatestSnapshot();
    if (!latest) return {};

    // 경과 틱 수 × 틱당 시간 = 예측에 쓸 시간 오프셋
    uint32_t tickDelta = (currentTick > latest->Tick)
                             ? (currentTick - latest->Tick) : 0;
    float dt = static_cast<float>(tickDelta) * deltaSeconds;

    PredictedPosition pred;
    pred.PosX = latest->PosX + latest->VelX * dt;
    pred.PosY = latest->PosY + latest->VelY * dt;
    pred.PosZ = latest->PosZ + latest->VelZ * dt;

    // Z 클램프 — 지면 아래로 예측되지 않도록
    if (pred.PosZ < 0.f) pred.PosZ = 0.f;

    return pred;
}

// ─────────────────────────────────────────────────────
//  보정 필요 여부
//
//  예측 위치와 서버 권위 위치 간 거리가 임계값 초과 시
//  클라이언트에 강제 위치 보정 패킷 전송이 필요
// ─────────────────────────────────────────────────────
bool DeadReckoning::NeedsCorrection(float actualX, float actualY,
                                    float actualZ) const
{
    const PositionSnapshot* latest = LatestSnapshot();
    if (!latest) return false;

    float dx = actualX - latest->PosX;
    float dy = actualY - latest->PosY;
    float dz = actualZ - latest->PosZ;
    float distSq = dx * dx + dy * dy + dz * dz;

    return distSq > (CORRECTION_THRESHOLD * CORRECTION_THRESHOLD);
}

// ─────────────────────────────────────────────────────
//  과거 위치 조회 — Lag Compensation
//
//  클라이언트가 "X ms 전 상태"에서 스킬을 쏜 경우
//  서버는 그 시점의 피격 대상 위치를 재현하여 히트 판정
//
//  활용:
//    클라이언트의 틱 번호 → GetHistoricalPosition(clientTick, ...) →
//    재현된 위치에서 히트박스 충돌 체크
// ─────────────────────────────────────────────────────
bool DeadReckoning::GetHistoricalPosition(uint32_t tick,
                                          PositionSnapshot& out) const
{
    for (int i = 0; i < m_count; ++i)
    {
        int idx = (m_head - i + HISTORY_SIZE) % HISTORY_SIZE;
        if (m_history[idx].Tick == tick)
        {
            out = m_history[idx];
            return true;
        }
    }
    return false;
}

void DeadReckoning::Reset()
{
    std::memset(m_history, 0, sizeof(m_history));
    m_head  = 0;
    m_count = 0;
}

const PositionSnapshot* DeadReckoning::LatestSnapshot() const
{
    if (m_count == 0) return nullptr;
    return &m_history[m_head];
}
