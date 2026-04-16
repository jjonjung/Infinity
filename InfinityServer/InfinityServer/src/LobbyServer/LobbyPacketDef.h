#pragma once

#include <cstdint>

#pragma pack(push, 1)

// ─────────────────────────────────────────────────────
//  로비 서버 전용 opcode (0x0100 ~ 0x01FF)
//
//  흐름:
//    클라이언트 → 로그인 서버(7000) → GameSessionToken 발급
//    클라이언트 → 로비 서버(8000) → 토큰 제시 → 방 입장 → 매치 시작
//    클라이언트 → 게임 서버(9000) → MatchToken 제시 → 게임 플레이
// ─────────────────────────────────────────────────────
constexpr uint16_t OP_LOBBY_CONNECT_REQ = 0x0101;  // C→S: 로비 진입 (GameSessionToken 제시)
constexpr uint16_t OP_LOBBY_CONNECT_RES = 0x0102;  // S→C: 진입 결과
constexpr uint16_t OP_ROOM_LIST_REQ     = 0x0103;  // C→S: 방 목록 조회
constexpr uint16_t OP_ROOM_LIST_RES     = 0x0104;  // S→C: 방 목록 응답
constexpr uint16_t OP_ROOM_CREATE_REQ   = 0x0105;  // C→S: 방 생성
constexpr uint16_t OP_ROOM_CREATE_RES   = 0x0106;  // S→C: 생성 결과
constexpr uint16_t OP_ROOM_JOIN_REQ     = 0x0107;  // C→S: 방 입장
constexpr uint16_t OP_ROOM_JOIN_RES     = 0x0108;  // S→C: 입장 결과
constexpr uint16_t OP_ROOM_LEAVE_REQ    = 0x0109;  // C→S: 방 퇴장
constexpr uint16_t OP_ROOM_LEAVE_RES    = 0x010A;  // S→C: 퇴장 확인
constexpr uint16_t OP_PLAYER_READY_REQ  = 0x010B;  // C→S: 준비 상태 토글
constexpr uint16_t OP_PLAYER_READY_NTFY = 0x010C;  // S→C: 준비 상태 브로드캐스트
constexpr uint16_t OP_MATCH_START_NTFY  = 0x010D;  // S→C: 게임 시작 — 게임 서버 주소 포함

// ─────────────────────────────────────────────────────
//  패킷 바디 구조체
// ─────────────────────────────────────────────────────

// C→S: 로비 진입 — 로그인 서버에서 발급받은 GameSessionToken 제시
struct LobbyConnectReqBody
{
    char game_session_token[64];
};

// S→C: 로비 진입 결과
struct LobbyConnectResBody
{
    uint8_t result;       // 1=성공, 0=실패 (토큰 만료/위조)
    int64_t user_id;
    char    nickname[32];
    char    message[64];
};

// 방 목록 항목 — RoomListResBody에 최대 16개 포함
struct RoomInfoBody
{
    uint32_t room_id;
    char     room_name[32];
    uint8_t  current_players;
    uint8_t  max_players;
    uint8_t  is_started;  // 이미 시작된 방은 입장 불가
};

// S→C: 방 목록
struct RoomListResBody
{
    uint8_t      room_count;
    RoomInfoBody rooms[16];
};

// C→S: 방 생성
struct RoomCreateReqBody
{
    char    room_name[32];
    uint8_t max_players;  // 2 또는 4
};

// S→C: 방 생성 결과
struct RoomCreateResBody
{
    uint8_t  result;
    uint32_t room_id;
    char     message[64];
};

// C→S: 방 입장
struct RoomJoinReqBody
{
    uint32_t room_id;
};

// S→C: 방 입장 결과 + 현재 방 상태
struct RoomJoinResBody
{
    uint8_t  result;
    uint32_t room_id;
    char     room_name[32];
    uint8_t  current_players;
    uint8_t  max_players;
    char     message[64];
};

// S→C: 준비 상태 변경 브로드캐스트 — 방 안 전원에게 전송
struct PlayerReadyNtfyBody
{
    int64_t user_id;
    char    nickname[32];
    uint8_t is_ready;
};

// S→C: 게임 시작 알림
//   - 접속할 게임 서버 IP:포트 포함
//   - match_token: 게임 서버 진입 시 사용하는 일회성 토큰 (GameSessionToken과 별개)
struct MatchStartNtfyBody
{
    char     game_server_ip[16];
    uint16_t game_server_port;
    char     match_token[64];
};

#pragma pack(pop)
