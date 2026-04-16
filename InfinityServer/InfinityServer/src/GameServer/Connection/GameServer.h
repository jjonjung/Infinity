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
class GameSession;

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

    // ─────────────────────────────────────────────────
    //  세션 레지스트리 — 인게임 채팅 브로드캐스트용
    //
    //  GameRoom은 게임 상태 담당, 세션 목록은 GameServer가 관리
    //  → 채팅처럼 "룸 내 모든 세션에 전송"이 필요한 경우 활용
    //
    //  스레드 안전:
    //    m_roomMutex로 보호 (룸 관리 락과 공유)
    //    BroadcastToRoom 내부에서 세션 포인터 스냅샷 후 락 해제 → 송신
    //    → LobbyRoom::Broadcast와 동일한 락 분리 전략
    // ─────────────────────────────────────────────────
    void RegisterSession  (uint32_t roomId, GameSession* sess);
    void UnregisterSession(uint32_t roomId, GameSession* sess);
    void BroadcastToRoom  (uint32_t roomId, uint16_t opcode,
                           const char* body, uint16_t bodySize);

    static GameServer& Get();

private:
    static DWORD WINAPI ClientThread(LPVOID param);

    uint16_t          m_port;
    SOCKET            m_listenSocket;
    std::atomic<bool> m_running;

    mutable std::mutex m_roomMutex;
    std::unordered_map<uint32_t, std::unique_ptr<GameRoom>>   m_rooms;
    std::unordered_map<std::string, uint32_t>                 m_tokenToRoomId;
    std::unordered_map<uint32_t, std::vector<GameSession*>>   m_roomSessions;  // 채팅 브로드캐스트용
    std::atomic<uint32_t> m_nextRoomId;

    static GameServer* s_instance;
};
