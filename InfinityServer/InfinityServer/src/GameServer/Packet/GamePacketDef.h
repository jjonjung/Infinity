#pragma once

#include <cstdint>

#pragma pack(push, 1)

// ─────────────────────────────────────────────────────
//  [게임 서버] Packet 구조 설계
//
//  직렬화 원칙:
//    - #pragma pack(push, 1) 으로 패딩 없는 바이너리 레이아웃
//    - 수치형: Little-Endian (x86/x64 기본)
//    - float: IEEE 754 단정밀도 (4바이트)
//    - 문자열: null-terminated char[] (동적 길이 없음)
//    - 타임스탬프: uint32_t (밀리초, 서버 틱 기준)
//
//  위치/방향:
//    InfinityFighter는 2D 격투 기반 → x, y 좌표 + yaw(회전)
//    z는 점프/공중 상태 표현에 사용
// ─────────────────────────────────────────────────────

// ── 접속 ──────────────────────────────────────────────

// C→S: 게임 서버 접속 요청 — 로비에서 발급받은 match_token 제시
struct GameConnectReqBody
{
    char match_token[64];
};

// S→C: 접속 결과 + 내 초기 상태
struct GameConnectResBody
{
    uint8_t  result;          // 1=성공
    int64_t  user_id;
    uint8_t  player_slot;     // 0~3, 룸 내 슬롯 번호
    float    spawn_x;
    float    spawn_y;
    float    spawn_z;
    char     character[32];   // 선택 캐릭터 이름
    char     message[64];
};

// ── 이동 ──────────────────────────────────────────────

// C→S: 클라이언트 이동 입력 — 위치가 아닌 "의도"를 전송
//   Authoritative 서버가 입력을 받아 위치를 직접 계산
struct GameMoveInputBody
{
    uint32_t tick;            // 클라이언트 로컬 틱 번호 (lag compensation용)
    float    dir_x;           // 이동 방향 벡터 (정규화, -1.0~1.0)
    float    dir_y;
    float    yaw;             // 캐릭터 회전 (라디안)
    uint8_t  jump;            // 1=점프 입력
    uint8_t  dash;            // 1=대시 입력
};

// S→C: 서버가 검증·계산한 이동 결과 — 룸 브로드캐스트
struct GameMoveBroadcastBody
{
    uint32_t server_tick;     // 서버 틱 번호
    int64_t  user_id;
    float    pos_x;           // 서버 권위 위치
    float    pos_y;
    float    pos_z;
    float    vel_x;           // 현재 속도 벡터 (Dead Reckoning용)
    float    vel_y;
    float    vel_z;
    float    yaw;
};

// ── 상태 스냅샷 ───────────────────────────────────────

// 플레이어 1명의 전체 상태 (스냅샷에 포함)
struct PlayerSnapshotEntry
{
    int64_t  user_id;
    float    pos_x;
    float    pos_y;
    float    pos_z;
    float    vel_x;
    float    vel_y;
    float    vel_z;
    float    yaw;
    int32_t  hp;
    int32_t  max_hp;
    uint8_t  state;           // PlayerStateFlag 비트필드
    uint16_t skill_cd[4];     // 스킬별 남은 쿨다운 (틱 단위)
};

// S→C: 전체 상태 스냅샷 — 주기적 전송 (5초마다) 또는 접속 직후
struct GameStateSnapshotBody
{
    uint32_t            server_tick;
    uint8_t             player_count;
    PlayerSnapshotEntry players[4];
};

// ── 델타 업데이트 ─────────────────────────────────────

// S→C: 직전 스냅샷 대비 변경된 필드만 전송 — 대역폭 절약
struct PlayerDeltaEntry
{
    int64_t  user_id;
    uint16_t dirty_flags;     // 변경된 필드 비트마스크 (GameDirtyFlag)
    int32_t  hp;              // dirty_flags & DIRTY_HP 일 때만 유효
    float    pos_x;           // dirty_flags & DIRTY_POS
    float    pos_y;
    float    pos_z;
    uint8_t  state;           // dirty_flags & DIRTY_STATE
};

struct GameStateDeltaBody
{
    uint32_t        server_tick;
    uint8_t         delta_count;
    PlayerDeltaEntry deltas[4];
};

// ── 전투 ──────────────────────────────────────────────

// C→S: 스킬 사용 요청
struct GameSkillUseReqBody
{
    uint32_t tick;
    uint8_t  skill_index;     // 0~3
    float    target_x;        // 조준 위치 (방향형 스킬)
    float    target_y;
};

// S→C: 스킬 발동 브로드캐스트
struct GameSkillBroadcastBody
{
    uint32_t server_tick;
    int64_t  caster_id;
    uint8_t  skill_index;
    float    origin_x;
    float    origin_y;
    float    origin_z;
    float    dir_x;
    float    dir_y;
};

// S→C: 피격 통보 (서버가 히트 판정)
struct GameHitNotifyBody
{
    uint32_t server_tick;
    int64_t  attacker_id;
    int64_t  victim_id;
    int32_t  damage;
    int32_t  remaining_hp;
    uint8_t  skill_index;
};

// S→C: 플레이어 사망
struct GamePlayerDiedBody
{
    int64_t killer_id;
    int64_t victim_id;
    uint32_t server_tick;
};

// S→C: 플레이어 부활
struct GamePlayerRespawnBody
{
    int64_t user_id;
    float   spawn_x;
    float   spawn_y;
    float   spawn_z;
    int32_t hp;
};

// ── 매치 종료 ─────────────────────────────────────────

// 결과 요약 (플레이어 1명)
struct MatchResultEntry
{
    int64_t user_id;
    char    nickname[32];
    int32_t kills;
    int32_t deaths;
    int32_t assists;
    int32_t damage_dealt;
    uint8_t is_winner;
};

// S→C: 매치 종료 알림
struct GameMatchEndBody
{
    uint32_t         server_tick;
    uint8_t          player_count;
    MatchResultEntry results[4];
};

// ── 핑 ────────────────────────────────────────────────

struct GamePingBody { uint32_t client_time_ms; };
struct GamePongBody { uint32_t client_time_ms; uint32_t server_time_ms; };

// ── 상태 플래그 ───────────────────────────────────────

enum PlayerStateFlag : uint8_t
{
    STATE_IDLE     = 0x00,
    STATE_MOVING   = 0x01,
    STATE_JUMPING  = 0x02,
    STATE_ATTACKING= 0x04,
    STATE_STUNNED  = 0x08,
    STATE_DEAD     = 0x10,
};

// 델타 업데이트용 더티 플래그
enum GameDirtyFlag : uint16_t
{
    DIRTY_HP       = 0x0001,
    DIRTY_POS      = 0x0002,
    DIRTY_VEL      = 0x0004,
    DIRTY_STATE    = 0x0008,
    DIRTY_SKILL_CD = 0x0010,
};

#pragma pack(pop)
