#include "LoginServer/LoginSession.h"
#include "LoginServer/LoginPacketHandler.h"
#include "Packet/Packet.h"
#include "Shared/Logging/Logger.h"

#include <cstring>
#include <string>

// 로그인 요청 바디는 작음 — 512B 제한
static constexpr uint16_t MAX_LOGIN_BODY = 512;

std::atomic<int> LoginSession::s_count = 0;

LoginSession::LoginSession(SOCKET socket, sockaddr_in addr)
    : m_socket(socket)
    , m_addr(addr)
{
    inet_ntop(AF_INET, &m_addr.sin_addr, m_clientIP, sizeof(m_clientIP));
    m_recvBuf.reserve(MAX_LOGIN_BODY);
    m_sendBuf.reserve(HEADER_SIZE + MAX_LOGIN_BODY);
    ++s_count;
}

LoginSession::~LoginSession()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    --s_count;
}

// ─────────────────────────────────────────────────────
//  단일 요청-응답 처리 후 연결 해제
//  로그인 서버는 keep-alive 없이 stateless로 동작
// ─────────────────────────────────────────────────────
void LoginSession::Run()
{
    PacketHeader header{};
    if (!RecvAll(reinterpret_cast<char*>(&header), static_cast<int>(HEADER_SIZE)))
    {
        Logger::Write(LogLevel::Warning, "login-session",
                      std::string(m_clientIP) + " 헤더 수신 실패");
        return;
    }

    if (header.body_size > MAX_LOGIN_BODY)
    {
        Logger::Write(LogLevel::Warning, "login-session",
                      std::string(m_clientIP) + " 비정상 패킷 크기 차단");
        return;
    }

    m_recvBuf.resize(header.body_size);
    if (header.body_size > 0 &&
        !RecvAll(m_recvBuf.data(), static_cast<int>(header.body_size)))
    {
        Logger::Write(LogLevel::Warning, "login-session",
                      std::string(m_clientIP) + " 바디 수신 실패");
        return;
    }

    LoginPacketHandler::Handle(*this, header.opcode,
                               m_recvBuf.data(), header.body_size);
    // 응답 전송 완료 → 소멸자에서 closesocket 자동 호출
}

void LoginSession::SendPacket(uint16_t opcode, const char* body, uint16_t bodySize)
{
    PacketHeader hdr;
    hdr.body_size = bodySize;
    hdr.opcode    = opcode;

    m_sendBuf.resize(HEADER_SIZE + bodySize);
    std::memcpy(m_sendBuf.data(), &hdr, HEADER_SIZE);
    if (bodySize > 0 && body)
        std::memcpy(m_sendBuf.data() + HEADER_SIZE, body, bodySize);

    send(m_socket, m_sendBuf.data(), static_cast<int>(m_sendBuf.size()), 0);
}

bool LoginSession::RecvAll(char* buf, int size)
{
    int recvd = 0;
    while (recvd < size)
    {
        int r = recv(m_socket, buf + recvd, size - recvd, 0);
        if (r <= 0) return false;
        recvd += r;
    }
    return true;
}

int LoginSession::GetActiveCount()
{
    return s_count.load();
}
