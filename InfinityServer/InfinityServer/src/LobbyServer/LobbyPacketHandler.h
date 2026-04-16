#pragma once

#include <cstdint>

class LobbySession;

// ─────────────────────────────────────────────────────
//  [로비 서버] 패킷 핸들러
//
//  처리 opcode (LobbyPacketDef.h 참조):
//    OP_LOBBY_CONNECT_REQ (0x0101) — GameSessionToken 검증 및 진입
//    OP_ROOM_LIST_REQ     (0x0103) — 방 목록 조회
//    OP_ROOM_CREATE_REQ   (0x0105) — 방 생성 + 자동 입장
//    OP_ROOM_JOIN_REQ     (0x0107) — 방 입장
//    OP_ROOM_LEAVE_REQ    (0x0109) — 방 퇴장
//    OP_PLAYER_READY_REQ  (0x010B) — 준비 토글 + 전원 준비 시 매치 시작
//    OP_CHAT_SEND_REQ     (0x010E) — 방 채팅 메시지 전송
// ─────────────────────────────────────────────────────
class LobbyPacketHandler
{
public:
    static void Handle(LobbySession& session, uint16_t opcode,
                       const char* body, uint16_t bodySize);

private:
    static void OnLobbyConnectReq (LobbySession& session, const char* body, uint16_t bodySize);
    static void OnRoomListReq     (LobbySession& session, const char* body, uint16_t bodySize);
    static void OnRoomCreateReq   (LobbySession& session, const char* body, uint16_t bodySize);
    static void OnRoomJoinReq     (LobbySession& session, const char* body, uint16_t bodySize);
    static void OnRoomLeaveReq    (LobbySession& session, const char* body, uint16_t bodySize);
    static void OnPlayerReadyReq  (LobbySession& session, const char* body, uint16_t bodySize);
    static void OnChatSendReq     (LobbySession& session, const char* body, uint16_t bodySize);
};
