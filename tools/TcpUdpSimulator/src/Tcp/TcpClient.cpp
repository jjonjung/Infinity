#include "../../include/Tcp/TcpClient.h"

#include <cstring>
#include <utility>

namespace TcpUdpSimulator
{

namespace
{
std::string MakeEndpointString(const std::string& host, int port)
{
    return host + ":" + std::to_string(port);
}
}

TcpClient::~TcpClient()
{
    Disconnect();
}

bool TcpClient::Connect(const std::string& host, int port)
{
    Disconnect();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* resultList = nullptr;
    const std::string portText = std::to_string(port);
    const int resolveResult = ::getaddrinfo(host.c_str(), portText.c_str(), &hints, &resultList);
    if (resolveResult != 0)
    {
        if (logSink_)
        {
            logSink_("TcpClient::Connect getaddrinfo failed: " + std::to_string(resolveResult));
        }
        return false;
    }

    for (addrinfo* node = resultList; node != nullptr; node = node->ai_next)
    {
        SocketHandle candidate(::socket(node->ai_family, node->ai_socktype, node->ai_protocol));
        if (!candidate.IsValid())
        {
            continue;
        }

        if (::connect(candidate.Get(), node->ai_addr, static_cast<int>(node->ai_addrlen)) == 0)
        {
            socket_ = std::move(candidate);
            ::freeaddrinfo(resultList);

            if (logSink_)
            {
                logSink_("TcpClient::Connect success -> " + MakeEndpointString(host, port));
            }
            return true;
        }
    }

    ::freeaddrinfo(resultList);

    if (logSink_)
    {
        logSink_(BuildSocketError("TcpClient::Connect"));
    }
    return false;
}

void TcpClient::Disconnect()
{
    if (socket_.IsValid() && logSink_)
    {
        logSink_("TcpClient::Disconnect");
    }

    socket_.Reset();
}

bool TcpClient::Send(const void* data, std::size_t size)
{
    if (!socket_.IsValid() || data == nullptr || size == 0)
    {
        return false;
    }

    const char* cursor = static_cast<const char*>(data);
    std::size_t remaining = size;

    while (remaining > 0)
    {
        const int sent = ::send(socket_.Get(), cursor, static_cast<int>(remaining), 0);
        if (sent == SOCKET_ERROR)
        {
            if (logSink_)
            {
                logSink_(BuildSocketError("TcpClient::Send"));
            }
            return false;
        }

        cursor += sent;
        remaining -= static_cast<std::size_t>(sent);
    }

    if (logSink_)
    {
        logSink_("TcpClient::Send bytes=" + std::to_string(size));
    }
    return true;
}

int TcpClient::Receive(void* buffer, std::size_t size)
{
    if (!socket_.IsValid() || buffer == nullptr || size == 0)
    {
        return SOCKET_ERROR;
    }

    const int received = ::recv(socket_.Get(), static_cast<char*>(buffer), static_cast<int>(size), 0);
    if (received == SOCKET_ERROR && logSink_)
    {
        logSink_(BuildSocketError("TcpClient::Receive"));
    }
    return received;
}

bool TcpClient::IsConnected() const
{
    return socket_.IsValid();
}

void TcpClient::SetLogSink(LogSink sink)
{
    logSink_ = std::move(sink);
}

} // namespace TcpUdpSimulator
