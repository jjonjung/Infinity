#pragma once

#include <cstdint>

// ─────────────────────────────────────────────────────
//  [게임 서버] 패킷 설계 — opcode 정의 (0x0200 ~ 0x02FF)
//
//  흐름:
//    로비 서버 → MatchStartNtfyBody(IP:포트, match_token) → 클라이언트
//    클라이언트 → OP_GAME_CONNECT_REQ(match_token) → 게임 서버
//    게임 서버 → 매치 진행 → OP_GAME_MATCH_END → 결과 DB 저장
//
//  방향 표기:
//    C→S: 클라이언트가 서버에 전송
//    S→C: 서버가 클라이언트에 전송 (특정 1명 or 룸 브로드캐스트)
// ─────────────────────────────────────────────────────

// 접속/초기화
constexpr uint16_t OP_GAME_CONNECT_REQ      = 0x0201;  // C→S: match_token 제시
constexpr uint16_t OP_GAME_CONNECT_RES      = 0x0202;  // S→C: 접속 결과 + 초기 상태

// 상태 동기화 (Authoritative Server → Client)
constexpr uint16_t OP_GAME_STATE_SNAPSHOT   = 0x0203;  // S→C: 전체 상태 스냅샷 (주기적)
constexpr uint16_t OP_GAME_STATE_DELTA      = 0x0204;  // S→C: 변경된 상태만 전송 (델타)

// 이동 동기화
constexpr uint16_t OP_GAME_MOVE_INPUT       = 0x0205;  // C→S: 방향키/이동 입력
constexpr uint16_t OP_GAME_MOVE_BROADCAST   = 0x0206;  // S→C: 검증된 이동 결과 브로드캐스트

// 전투 액션
constexpr uint16_t OP_GAME_SKILL_USE_REQ    = 0x0207;  // C→S: 스킬 사용 요청
constexpr uint16_t OP_GAME_SKILL_BROADCAST  = 0x0208;  // S→C: 스킬 발동 브로드캐스트
constexpr uint16_t OP_GAME_HIT_NOTIFY       = 0x0209;  // S→C: 피격 통보 (서버 계산)

// 플레이어 상태 이벤트
constexpr uint16_t OP_GAME_PLAYER_DIED      = 0x020A;  // S→C: 플레이어 사망
constexpr uint16_t OP_GAME_PLAYER_RESPAWN   = 0x020B;  // S→C: 플레이어 부활

// 매치 관리
constexpr uint16_t OP_GAME_MATCH_END        = 0x020C;  // S→C: 매치 종료 + 결과

// 지연 측정
constexpr uint16_t OP_GAME_PING             = 0x020D;  // C→S: 핑 요청
constexpr uint16_t OP_GAME_PONG             = 0x020E;  // S→C: 핑 응답

// 인게임 채팅
constexpr uint16_t OP_GAME_CHAT_REQ         = 0x020F;  // C→S: 채팅 메시지 전송
constexpr uint16_t OP_GAME_CHAT_NTFY        = 0x0210;  // S→C: 채팅 메시지 브로드캐스트
