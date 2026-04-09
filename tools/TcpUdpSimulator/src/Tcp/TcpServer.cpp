#include "../../include/Tcp/TcpServer.h"

#include <utility>

namespace TcpUdpSimulator
{

TcpServer::~TcpServer()
{
    Stop();
}

bool TcpServer::Start(int port)
{
    Stop();

    SocketHandle candidate(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!candidate.IsValid())
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("TcpServer::socket"));
        }
        return false;
    }

    BOOL reuse = TRUE;
    ::setsockopt(candidate.Get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<u_short>(port));

    if (::bind(candidate.Get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("TcpServer::bind"));
        }
        return false;
    }

    if (::listen(candidate.Get(), SOMAXCONN) == SOCKET_ERROR)
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("TcpServer::listen"));
        }
        return false;
    }

    if (!SetNonBlocking(candidate.Get(), true))
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("TcpServer::ioctlsocket"));
        }
        return false;
    }

    listenSocket_ = std::move(candidate);
    lastReceiveSize_ = 0;

    if (logSink_)
    {
        logSink_("TcpServer::Start port=" + std::to_string(port));
    }
    return true;
}

void TcpServer::Stop()
{
    if ((listenSocket_.IsValid() || clientSocket_.IsValid()) && logSink_)
    {
        logSink_("TcpServer::Stop");
    }

    lastReceiveSize_ = 0;
    clientSocket_.Reset();
    listenSocket_.Reset();
}

void TcpServer::TickAccept()
{
    if (!listenSocket_.IsValid() || clientSocket_.IsValid())
    {
        return;
    }

    sockaddr_in clientAddress{};
    int clientAddressLength = sizeof(clientAddress);
    SOCKET accepted = ::accept(listenSocket_.Get(), reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);
    if (accepted == INVALID_SOCKET)
    {
        const int error = ::WSAGetLastError();
        if (error != WSAEWOULDBLOCK && logSink_)
        {
            logSink_("TcpServer::accept failed: " + std::to_string(error));
        }
        return;
    }

    clientSocket_.Reset(accepted);
    SetNonBlocking(clientSocket_.Get(), true);

    if (logSink_)
    {
        logSink_("TcpServer::TickAccept accepted client");
    }
}

void TcpServer::TickReceive()
{
    lastReceiveSize_ = 0;

    if (!clientSocket_.IsValid())
    {
        return;
    }

    char buffer[1024]{};
    const int received = ::recv(clientSocket_.Get(), buffer, static_cast<int>(sizeof(buffer)), 0);
    if (received > 0)
    {
        lastReceiveSize_ = received;
        if (logSink_)
        {
            logSink_("TcpServer::TickReceive bytes=" + std::to_string(received));
        }
        return;
    }

    if (received == 0)
    {
        if (logSink_)
        {
            logSink_("TcpServer::TickReceive peer disconnected");
        }
        clientSocket_.Reset();
        return;
    }

    const int error = ::WSAGetLastError();
    if (error != WSAEWOULDBLOCK)
    {
        if (logSink_)
        {
            logSink_("TcpServer::TickReceive failed: " + std::to_string(error));
        }
        clientSocket_.Reset();
    }
}

bool TcpServer::HasClient() const
{
    return clientSocket_.IsValid();
}

int TcpServer::GetLastReceiveSize() const
{
    return lastReceiveSize_;
}

void TcpServer::SetLogSink(LogSink sink)
{
    logSink_ = std::move(sink);
}

} // namespace TcpUdpSimulator
