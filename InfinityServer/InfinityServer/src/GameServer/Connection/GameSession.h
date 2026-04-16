#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>
#include "Shared/Network/ISession.h"

class GameRoom;

// ─────────────────────────────────────────────────────
//  [게임 서버] 플레이어 접속 세션
//
//  접속 절차:
//    1. OP_GAME_CONNECT_REQ: match_token 제시
//    2. 서버가 토큰 검증 → 유효하면 GameRoom에 참가
//    3. 이후 게임 패킷 루프 (이동 입력, 스킬 사용 등)
//    4. 매치 종료 또는 연결 해제 → 룸에서 제거
//
//  스레드 안전:
//    SendPacket은 서버 틱 스레드에서도 호출 가능 → m_sendMutex
// ─────────────────────────────────────────────────────
class GameSession : public ISession
{
public:
    GameSession(SOCKET socket, sockaddr_in addr);
    ~GameSession();

    void Run();
    void SendPacket(uint16_t opcode, const char* body, uint16_t bodySize) override;

    void     SetUserId(int64_t id)   { m_userId = id; }
    void     SetRoom(GameRoom* room) { m_room = room; }
    void     SetAuthenticated()      { m_authenticated = true; }

    int64_t    GetUserId()       const { return m_userId; }
    GameRoom*  GetRoom()         const { return m_room; }
    bool       IsAuthenticated() const { return m_authenticated; }

    static int GetActiveCount();

private:
    bool RecvAll(char* buf, int size);

    SOCKET            m_socket;
    sockaddr_in       m_addr;
    char              m_clientIP[INET_ADDRSTRLEN];
    std::vector<char> m_recvBuf;
    std::vector<char> m_sendBuf;
    mutable std::mutex m_sendMutex;

    bool      m_authenticated = false;
    int64_t   m_userId        = 0;
    GameRoom* m_room          = nullptr;

    static std::atomic<int> s_count;
};
