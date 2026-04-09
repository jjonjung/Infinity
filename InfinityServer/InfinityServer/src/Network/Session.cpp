#include "Network/Session.h"
#include "Packet/Packet.h"
#include "Packet/PacketHandler.h"

#include <iostream>
#include <vector>
#include <cstring>

// ─── 최대 허용 바디 크기 (비정상 패킷 차단) ──────────
static constexpr uint16_t MAX_BODY_SIZE = 4096;

std::atomic<int> Session::s_activeSessions = 0;

Session::Session(SOCKET socket, sockaddr_in addr)
    : m_socket(socket)
    , m_addr(addr)
{
    inet_ntop(AF_INET, &m_addr.sin_addr, m_clientIP, sizeof(m_clientIP));
    m_receiveBuffer.reserve(MAX_BODY_SIZE);
    m_sendBuffer.reserve(HEADER_SIZE + MAX_BODY_SIZE);
    ++s_activeSessions;
}

Session::~Session()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    --s_activeSessions;
}

// ─────────────────────────────────────────────────────
//  수신 루프
// ─────────────────────────────────────────────────────
void Session::Run()
{
    PacketHeader header{};

    while (true)
    {
        // 1. 헤더 수신 (4 bytes 고정)
        if (!RecvAll(reinterpret_cast<char*>(&header), static_cast<int>(HEADER_SIZE)))
        {
            std::cout << "[세션] " << m_clientIP << " 연결 종료\n";
            break;
        }

        const uint16_t bodySize = header.body_size;
        const uint16_t opcode   = header.opcode;

        // 비정상 크기 차단
        if (bodySize > MAX_BODY_SIZE)
        {
            std::cerr << "[세션] " << m_clientIP
                      << " 비정상 패킷 크기 " << bodySize << " — 연결 차단\n";
            break;
        }

        // 2. 바디 수신
        m_receiveBuffer.resize(bodySize);
        if (bodySize > 0 && !RecvAll(m_receiveBuffer.data(), static_cast<int>(bodySize)))
        {
            std::cout << "[세션] " << m_clientIP << " 바디 수신 실패\n";
            break;
        }

        // 3. 패킷 처리
        PacketHandler::Handle(*this, opcode, m_receiveBuffer.data(), bodySize);
    }
}

// ─────────────────────────────────────────────────────
//  정확히 size bytes 수신 (short read 방지)
// ─────────────────────────────────────────────────────
bool Session::RecvAll(char* buf, int size)
{
    int received = 0;
    while (received < size)
    {
        int ret = recv(m_socket, buf + received, size - received, 0);
        if (ret <= 0) return false;  // 0=연결 종료, -1=오류
        received += ret;
    }
    return true;
}

// ─────────────────────────────────────────────────────
//  헤더 + 바디를 단일 버퍼로 합쳐서 전송
// ─────────────────────────────────────────────────────
void Session::SendPacket(uint16_t opcode, const char* body, uint16_t bodySize)
{
    PacketHeader header;
    header.body_size = bodySize;
    header.opcode    = opcode;

    m_sendBuffer.resize(HEADER_SIZE + bodySize);
    std::memcpy(m_sendBuffer.data(), &header, HEADER_SIZE);
    if (bodySize > 0 && body)
        std::memcpy(m_sendBuffer.data() + HEADER_SIZE, body, bodySize);

    send(m_socket, m_sendBuffer.data(), static_cast<int>(m_sendBuffer.size()), 0);
}

int Session::GetActiveSessionCount()
{
    return s_activeSessions.load();
}
