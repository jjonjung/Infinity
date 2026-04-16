#include "LobbyServer/LobbyPacketHandler.h"
#include "LobbyServer/LobbyPacketDef.h"
#include "LobbyServer/LobbyRoom.h"
#include "LobbyServer/LobbyServer.h"
#include "LobbyServer/LobbySession.h"
#include "Bootstrap/ServerRuntime.h"
#include "Shared/Logging/Logger.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_map>

// 게임 서버 주소 (실제 환경에서는 ServerConfig에서 읽어야 함)
static constexpr const char*  GAME_SERVER_IP   = "127.0.0.1";
static constexpr uint16_t     GAME_SERVER_PORT  = 9000;

void LobbyPacketHandler::Handle(LobbySession& session, uint16_t opcode,
                                const char* body, uint16_t bodySize)
{
    // 로비 접속 요청은 인증 전에도 허용 (인증 테이블에 넣지 않음)
    if (opcode == OP_LOBBY_CONNECT_REQ)
    {
        OnLobbyConnectReq(session, body, bodySize);
        return;
    }

    if (!session.IsAuthenticated())
    {
        Logger::Write(LogLevel::Warning, "lobby-handler",
                      "미인증 세션에서 opcode: " + std::to_string(opcode));
        return;
    }

    // ── [OCP 수정] switch → 함수 포인터 맵 ─────────────
    //
    //  이전: 새 opcode 추가 = Handle() switch 수정 (기존 코드 변경)
    //  이후: 새 opcode 추가 = s_table에 항목 추가만 (기존 코드 불변)
    //
    //  static local → 프로그램 수명 동안 1회 초기화, lock-free 이후 읽기
    using HandlerFn = void(*)(LobbySession&, const char*, uint16_t);
    static const std::unordered_map<uint16_t, HandlerFn> s_table = {
        { OP_ROOM_LIST_REQ,    OnRoomListReq    },
        { OP_ROOM_CREATE_REQ,  OnRoomCreateReq  },
        { OP_ROOM_JOIN_REQ,    OnRoomJoinReq    },
        { OP_ROOM_LEAVE_REQ,   OnRoomLeaveReq   },
        { OP_PLAYER_READY_REQ, OnPlayerReadyReq },
    };

    auto it = s_table.find(opcode);
    if (it != s_table.end())
        it->second(session, body, bodySize);
    else
        Logger::Write(LogLevel::Warning, "lobby-handler",
                      "알 수 없는 opcode: " + std::to_string(opcode));
}

// ─────────────────────────────────────────────────────
//  로비 진입 — GameSessionToken 검증
// ─────────────────────────────────────────────────────
void LobbyPacketHandler::OnLobbyConnectReq(LobbySession& session,
                                           const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(LobbyConnectReqBody)) return;
    const auto* req = reinterpret_cast<const LobbyConnectReqBody*>(body);

    auto result = ServerRuntime::Get().GetAuthService()
                      .ValidateGameSessionToken(req->game_session_token);

    LobbyConnectResBody res{};
    if (result.Success)
    {
        session.SetAuthenticated(result.Value.UserId, result.Value.Nickname);
        res.result  = 1;
        res.user_id = result.Value.UserId;
        std::strncpy(res.nickname, result.Value.Nickname.c_str(), sizeof(res.nickname) - 1);
        std::strncpy(res.message, "로비 진입 성공", sizeof(res.message) - 1);
    }
    else
    {
        res.result = 0;
        std::strncpy(res.message, "토큰 검증 실패", sizeof(res.message) - 1);
    }

    session.SendPacket(OP_LOBBY_CONNECT_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}

// ─────────────────────────────────────────────────────
//  방 목록 조회
// ─────────────────────────────────────────────────────
void LobbyPacketHandler::OnRoomListReq(LobbySession& session,
                                       const char* /*body*/, uint16_t /*bodySize*/)
{
    auto rooms = LobbyServer::Get().GetAllRooms();

    RoomListResBody res{};
    res.room_count = static_cast<uint8_t>(std::min(rooms.size(), size_t(16)));
    for (uint8_t i = 0; i < res.room_count; ++i)
    {
        LobbyRoom* r = rooms[i];
        res.rooms[i].room_id         = r->GetId();
        res.rooms[i].current_players = r->GetPlayerCount();
        res.rooms[i].max_players     = r->GetMaxPlayers();
        res.rooms[i].is_started      = r->IsStarted() ? 1 : 0;
        std::strncpy(res.rooms[i].room_name, r->GetName(),
                     sizeof(res.rooms[i].room_name) - 1);
    }

    session.SendPacket(OP_ROOM_LIST_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}

// ─────────────────────────────────────────────────────
//  방 생성 — 생성자는 자동으로 입장
// ─────────────────────────────────────────────────────
void LobbyPacketHandler::OnRoomCreateReq(LobbySession& session,
                                         const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(RoomCreateReqBody)) return;
    const auto* req = reinterpret_cast<const RoomCreateReqBody*>(body);

    if (session.GetRoom())
    {
        RoomCreateResBody res{};
        res.result = 0;
        std::strncpy(res.message, "이미 방에 참가 중", sizeof(res.message) - 1);
        session.SendPacket(OP_ROOM_CREATE_RES,
                           reinterpret_cast<const char*>(&res), sizeof(res));
        return;
    }

    uint8_t maxPlayers = (req->max_players == 2 || req->max_players == 4)
                             ? req->max_players : 4;
    uint32_t roomId = LobbyServer::Get().CreateRoom(req->room_name, maxPlayers);
    LobbyRoom* room = LobbyServer::Get().FindRoom(roomId);
    room->AddPlayer(&session);
    session.SetRoom(room);

    RoomCreateResBody res{};
    res.result  = 1;
    res.room_id = roomId;
    std::strncpy(res.message, "방 생성 성공", sizeof(res.message) - 1);
    session.SendPacket(OP_ROOM_CREATE_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}

// ─────────────────────────────────────────────────────
//  방 입장
// ─────────────────────────────────────────────────────
void LobbyPacketHandler::OnRoomJoinReq(LobbySession& session,
                                       const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(RoomJoinReqBody)) return;
    const auto* req = reinterpret_cast<const RoomJoinReqBody*>(body);

    LobbyRoom* room = LobbyServer::Get().FindRoom(req->room_id);
    RoomJoinResBody res{};

    if (!room || room->IsStarted() || room->IsFull())
    {
        res.result = 0;
        std::strncpy(res.message, "입장 불가 (방 없음 / 이미 시작 / 정원 초과)",
                     sizeof(res.message) - 1);
    }
    else if (session.GetRoom())
    {
        res.result = 0;
        std::strncpy(res.message, "이미 방에 참가 중", sizeof(res.message) - 1);
    }
    else
    {
        room->AddPlayer(&session);
        session.SetRoom(room);
        res.result          = 1;
        res.room_id         = room->GetId();
        res.current_players = room->GetPlayerCount();
        res.max_players     = room->GetMaxPlayers();
        std::strncpy(res.room_name, room->GetName(), sizeof(res.room_name) - 1);
        std::strncpy(res.message, "입장 성공", sizeof(res.message) - 1);
    }

    session.SendPacket(OP_ROOM_JOIN_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}

// ─────────────────────────────────────────────────────
//  방 퇴장
// ─────────────────────────────────────────────────────
void LobbyPacketHandler::OnRoomLeaveReq(LobbySession& session,
                                        const char* /*body*/, uint16_t /*bodySize*/)
{
    if (LobbyRoom* room = session.GetRoom())
    {
        room->RemovePlayer(session.GetUserId());
        session.ClearRoom();
        LobbyServer::Get().RemoveRoomIfEmpty(room->GetId());
    }
    session.SendPacket(OP_ROOM_LEAVE_RES, nullptr, 0);
}

// ─────────────────────────────────────────────────────
//  준비 상태 토글
//  전원 준비 완료 시 게임 서버 주소 브로드캐스트
// ─────────────────────────────────────────────────────
void LobbyPacketHandler::OnPlayerReadyReq(LobbySession& session,
                                          const char* /*body*/, uint16_t /*bodySize*/)
{
    LobbyRoom* room = session.GetRoom();
    if (!room) return;

    room->SetReady(session.GetUserId(), true);

    PlayerReadyNtfyBody ntfy{};
    ntfy.user_id  = session.GetUserId();
    ntfy.is_ready = 1;
    std::strncpy(ntfy.nickname, session.GetNickname(), sizeof(ntfy.nickname) - 1);
    room->Broadcast(OP_PLAYER_READY_NTFY,
                    reinterpret_cast<const char*>(&ntfy), sizeof(ntfy));

    if (room->AllReady())
        room->StartMatch(GAME_SERVER_IP, GAME_SERVER_PORT);
}
