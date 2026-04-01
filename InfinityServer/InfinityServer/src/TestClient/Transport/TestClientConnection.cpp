#include "TestClient/Transport/TestClientConnection.h"

#include <cstring>

TestClientConnection::TestClientConnection()
    : m_socket(INVALID_SOCKET)
    , m_wsaStarted(false)
{
}

TestClientConnection::~TestClientConnection()
{
    Disconnect();
}

bool TestClientConnection::Connect(const std::string& host, uint16_t port, std::string& errorMessage)
{
    Disconnect();

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        errorMessage = "WSAStartup failed";
        return false;
    }
    m_wsaStarted = true;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        errorMessage = "socket creation failed";
        Disconnect();
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1)
    {
        errorMessage = "invalid host address";
        Disconnect();
        return false;
    }

    if (connect(m_socket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        errorMessage = "connect failed";
        Disconnect();
        return false;
    }

    return true;
}

void TestClientConnection::Disconnect()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    if (m_wsaStarted)
    {
        WSACleanup();
        m_wsaStarted = false;
    }
}

bool TestClientConnection::IsConnected() const
{
    return m_socket != INVALID_SOCKET;
}

bool TestClientConnection::SendPacket(uint16_t opcode, const void* body, uint16_t bodySize, std::string& errorMessage)
{
    if (!IsConnected())
    {
        errorMessage = "client is not connected";
        return false;
    }

    PacketHeader header{};
    header.body_size = bodySize;
    header.opcode = opcode;

    std::vector<char> packet(HEADER_SIZE + bodySize);
    std::memcpy(packet.data(), &header, HEADER_SIZE);
    if (bodySize > 0 && body != nullptr)
    {
        std::memcpy(packet.data() + HEADER_SIZE, body, bodySize);
    }

    const int sent = send(m_socket, packet.data(), static_cast<int>(packet.size()), 0);
    if (sent != static_cast<int>(packet.size()))
    {
        errorMessage = "send failed";
        return false;
    }

    return true;
}

bool TestClientConnection::ReceivePacket(PacketEnvelope& packet, std::string& errorMessage)
{
    PacketHeader header{};
    if (!RecvAll(reinterpret_cast<char*>(&header), static_cast<int>(HEADER_SIZE), errorMessage))
    {
        return false;
    }

    packet.Opcode = header.opcode;
    packet.Body.assign(header.body_size, '\0');

    if (header.body_size > 0 &&
        !RecvAll(packet.Body.data(), static_cast<int>(header.body_size), errorMessage))
    {
        return false;
    }

    return true;
}

bool TestClientConnection::RecvAll(char* buffer, int size, std::string& errorMessage)
{
    int received = 0;
    while (received < size)
    {
        const int ret = recv(m_socket, buffer + received, size - received, 0);
        if (ret <= 0)
        {
            errorMessage = "recv failed";
            return false;
        }

        received += ret;
    }

    return true;
}
