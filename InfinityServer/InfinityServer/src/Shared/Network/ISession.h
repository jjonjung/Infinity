#pragma once

#include <cstdint>

// ─────────────────────────────────────────────────────
//  [SOLID] ISession — 세션 공통 인터페이스
//
//  적용 원칙:
//    LSP — LoginSession / LobbySession / GameSession을
//          ISession으로 치환 가능 (동일 시그니처 보장)
//    ISP — 세션을 "받는 쪽"은 SendPacket만 알면 충분
//          → 인증 상태나 방 정보 같은 구체적 멤버에 의존하지 않음
//    DIP — Broadcast, PacketHandler 등 상위 모듈이
//          구체 세션 타입 대신 ISession*에 의존 가능
//
//  사용 예:
//    void Broadcast(ISession** sessions, int count, ...);
//    // LoginSession, LobbySession, GameSession 모두 전달 가능
// ─────────────────────────────────────────────────────
class ISession
{
public:
    virtual ~ISession() = default;

    // 헤더 + 바디를 조립하여 TCP 소켓으로 전송
    virtual void SendPacket(uint16_t opcode,
                            const char* body,
                            uint16_t bodySize) = 0;
};
