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

class GameRoom;

// ─────────────────────────────────────────────────────
//  [게임 서버] 게임 플레이 TCP 서버 (포트 9000)
//
//  역할:
//    - 로비에서 match_token 발급 후 접속한 플레이어 수용
//    - GameRoom 단위로 게임 상태 격리
//    - 룸별 ServerTick 스레드 구동
//
//  GameRoom 생명주기:
//    첫 번째 플레이어 접속 → CreateRoom (match_token 키)
//    이후 플레이어들 → FindRoomByToken → 같은 룸 입장
//    매치 종료 → DB 저장 → 룸 제거
// ─────────────────────────────────────────────────────
class GameServer
{
public:
    explicit GameServer(uint16_t port);
    ~GameServer();

    bool Init();
    void Run();
    void Stop();

    // 룸 관리
    GameRoom* FindOrCreateRoom(const std::string& matchToken);
    GameRoom* FindRoom(uint32_t roomId);
    void      RemoveRoom(uint32_t roomId);

    static GameServer& Get();

private:
    static DWORD WINAPI ClientThread(LPVOID param);

    uint16_t          m_port;
    SOCKET            m_listenSocket;
    std::atomic<bool> m_running;

    mutable std::mutex m_roomMutex;
    std::unordered_map<uint32_t, std::unique_ptr<GameRoom>> m_rooms;
    std::unordered_map<std::string, uint32_t>               m_tokenToRoomId;
    std::atomic<uint32_t> m_nextRoomId;

    static GameServer* s_instance;
};
