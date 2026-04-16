#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include "Shared/Network/ISession.h"

class LobbyRoom;

// ─────────────────────────────────────────────────────
//  [로비 서버] 클라이언트 세션 — 로비에 머무는 동안 연결 유지
//
//  인증:
//    접속 직후 OP_LOBBY_CONNECT_REQ로 GameSessionToken 검증
//    검증 성공 전까지 다른 패킷은 LobbyPacketHandler에서 차단
//
//  방 연동:
//    방 입장 시 SetRoom(), 퇴장/연결 해제 시 ClearRoom()
//    소멸자에서 방에 자신을 자동 제거하여 dangling pointer 방지
//
//  스레드 안전:
//    SendPacket은 LobbyRoom::Broadcast에서 다른 세션의 스레드가 호출 가능
//    → m_sendMutex로 send 버퍼 보호
// ─────────────────────────────────────────────────────
class LobbySession : public ISession
{
public:
    LobbySession(SOCKET socket, sockaddr_in addr);
    ~LobbySession();

    void Run();
    void SendPacket(uint16_t opcode, const char* body, uint16_t bodySize) override;

    void        SetAuthenticated(int64_t userId, const std::string& nickname);
    bool        IsAuthenticated() const { return m_authenticated; }
    int64_t     GetUserId()       const { return m_userId; }
    const char* GetNickname()     const { return m_nickname; }

    void       SetRoom(LobbyRoom* room);
    void       ClearRoom();
    LobbyRoom* GetRoom() const { return m_room; }

    static int GetActiveCount();

private:
    bool RecvAll(char* buf, int size);

    SOCKET            m_socket;
    sockaddr_in       m_addr;
    char              m_clientIP[INET_ADDRSTRLEN];
    std::vector<char> m_recvBuf;
    std::vector<char> m_sendBuf;
    mutable std::mutex m_sendMutex;

    bool       m_authenticated = false;
    int64_t    m_userId        = 0;
    char       m_nickname[32]  = {};
    LobbyRoom* m_room          = nullptr;   // 비소유 포인터

    static std::atomic<int> s_count;
};
