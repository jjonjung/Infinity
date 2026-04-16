#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class LobbyRoom;

// ─────────────────────────────────────────────────────
//  [로비 서버] 방 관리 + 매치 시작 TCP 서버 (포트 8000)
//
//  역할:
//    - 로그인 서버에서 GameSessionToken 발급 후 접속
//    - 방 목록 조회 / 생성 / 입장 / 퇴장
//    - 전원 준비 시 게임 서버 IP:포트 + match_token 브로드캐스트
//
//  방 생명주기:
//    CreateRoom → AddPlayer → SetReady × N → StartMatch → 방 자동 제거
//    마지막 플레이어 퇴장 시 RemoveRoomIfEmpty 호출로 정리
//
//  싱글톤:
//    LobbyPacketHandler에서 LobbyServer::Get()으로 방 조작
// ─────────────────────────────────────────────────────
class LobbyServer
{
public:
    explicit LobbyServer(uint16_t port);
    ~LobbyServer();

    bool Init();
    void Run();
    void Stop();

    uint32_t                   CreateRoom(const std::string& name, uint8_t maxPlayers);
    LobbyRoom*                 FindRoom(uint32_t roomId);
    void                       RemoveRoomIfEmpty(uint32_t roomId);
    std::vector<LobbyRoom*>    GetAllRooms();

    static LobbyServer& Get();

private:
    static DWORD WINAPI ClientThread(LPVOID param);

    uint16_t          m_port;
    SOCKET            m_listenSocket;
    std::atomic<bool> m_running;

    mutable std::mutex m_roomMutex;
    std::unordered_map<uint32_t, std::unique_ptr<LobbyRoom>> m_rooms;
    std::atomic<uint32_t> m_nextRoomId;

    static LobbyServer* s_instance;
};
