#pragma once

#include <cstdint>

class LoginSession;

// ─────────────────────────────────────────────────────
//  [로그인 서버] 패킷 핸들러 — 인증 전용 opcode 라우터
//
//  등록 opcode (기존 Packet.h 상수 재사용):
//    OP_REGISTER_REQ     (0x0006) — 로컬 계정 회원가입
//    OP_LOGIN_REQ        (0x0001) — 이메일/비밀번호 로그인
//    OP_SOCIAL_LOGIN_REQ (0x0003) — 구글/애플 OAuth 로그인
//
//  성공 응답에 game_session_token 포함
//  → 클라이언트는 이 토큰으로 로비 서버(포트 8000) 접속
// ─────────────────────────────────────────────────────
class LoginPacketHandler
{
public:
    static void Handle(LoginSession& session, uint16_t opcode,
                       const char* body, uint16_t bodySize);

private:
    static void OnRegisterReq    (LoginSession& session, const char* body, uint16_t bodySize);
    static void OnLoginReq       (LoginSession& session, const char* body, uint16_t bodySize);
    static void OnSocialLoginReq (LoginSession& session, const char* body, uint16_t bodySize);
};
