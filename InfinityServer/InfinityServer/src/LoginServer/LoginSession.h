#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <cstdint>
#include <vector>
#include "Shared/Network/ISession.h"

// ─────────────────────────────────────────────────────
//  [로그인 서버] 클라이언트 단위 세션
//
//  생명주기:
//    1. 클라이언트 TCP 접속 → 생성
//    2. 헤더(4B) + 바디 수신
//    3. LoginPacketHandler 처리 → 응답 전송
//    4. 응답 완료 즉시 소멸 (keep-alive 없음)
//
//  스레드 안전성:
//    각 세션은 전용 스레드 1개에서만 Run() 호출 → 별도 동기화 불필요
//    SendPacket은 Run() 내부에서만 호출됨
// ─────────────────────────────────────────────────────
class LoginSession : public ISession
{
public:
    LoginSession(SOCKET socket, sockaddr_in addr);
    ~LoginSession();

    void Run();
    void SendPacket(uint16_t opcode, const char* body, uint16_t bodySize) override;

    static int GetActiveCount();

private:
    bool RecvAll(char* buf, int size);

    SOCKET            m_socket;
    sockaddr_in       m_addr;
    char              m_clientIP[INET_ADDRSTRLEN];
    std::vector<char> m_recvBuf;
    std::vector<char> m_sendBuf;

    static std::atomic<int> s_count;
};
