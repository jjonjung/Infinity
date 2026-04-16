#pragma once

#include <cstdint>

// ─────────────────────────────────────────────────────
//  [게임 서버] Dead Reckoning / 보간 — 클라이언트 예측 지원
//
//  Dead Reckoning이란:
//    마지막으로 알고 있는 위치 + 속도로 현재 위치를 예측
//    네트워크 지연 동안 다른 플레이어가 "멈춰 보이지 않도록" 처리
//
//  서버에서의 역할:
//    1. 각 플레이어의 PredictedPosition을 서버 틱마다 계산
//    2. 실제 위치(권위 값)와 예측 위치 간의 오차 측정
//    3. 오차가 임계값 초과 시 → 클라이언트에 강제 보정 전송
//
//  클라이언트에서의 역할 (참고):
//    - 서버에서 마지막으로 받은 위치 + 속도로 로컬 예측
//    - 새 권위 위치 도착 시 부드럽게 블렌딩 (Interpolation)
//
//  스냅샷 기록:
//    - 최근 N 틱의 위치 이력을 유지
//    - 히트 판정 시 클라이언트 시점의 위치를 재현 (Lag Compensation)
// ─────────────────────────────────────────────────────

struct PositionSnapshot
{
    uint32_t Tick  = 0;
    float    PosX  = 0.f;
    float    PosY  = 0.f;
    float    PosZ  = 0.f;
    float    VelX  = 0.f;
    float    VelY  = 0.f;
    float    VelZ  = 0.f;
};

struct PredictedPosition
{
    float PosX = 0.f;
    float PosY = 0.f;
    float PosZ = 0.f;
};

class DeadReckoning
{
public:
    static constexpr int  HISTORY_SIZE         = 64;    // 유지할 스냅샷 수
    static constexpr float CORRECTION_THRESHOLD = 50.f; // 보정 트리거 거리(units)

    DeadReckoning();

    // 권위 상태 수신 시 스냅샷 추가
    void AddSnapshot(const PositionSnapshot& snap);

    // 현재 틱에서 위치 예측 (서버 틱 간격 = deltaSeconds)
    PredictedPosition Predict(uint32_t currentTick, float deltaSeconds) const;

    // 보정 필요 여부 — 예측 위치와 실제 위치 간 오차 검사
    bool NeedsCorrection(float actualX, float actualY, float actualZ) const;

    // 특정 틱의 과거 위치 반환 (Lag Compensation용)
    bool GetHistoricalPosition(uint32_t tick, PositionSnapshot& out) const;

    void Reset();

private:
    // 링 버퍼로 최근 HISTORY_SIZE개의 스냅샷 관리
    PositionSnapshot m_history[HISTORY_SIZE];
    int              m_head  = 0;    // 가장 최근 스냅샷 인덱스
    int              m_count = 0;    // 유효한 스냅샷 수

    const PositionSnapshot* LatestSnapshot() const;
};
