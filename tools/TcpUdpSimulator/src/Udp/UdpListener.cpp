#include "../../include/Udp/UdpListener.h"

#include <utility>

namespace TcpUdpSimulator
{

UdpListener::~UdpListener()
{
    Close();
}

bool UdpListener::Open(int port)
{
    Close();

    SocketHandle candidate(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (!candidate.IsValid())
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("UdpListener::socket"));
        }
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<u_short>(port));

    if (::bind(candidate.Get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("UdpListener::bind"));
        }
        return false;
    }

    if (!SetNonBlocking(candidate.Get(), true))
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("UdpListener::ioctlsocket"));
        }
        return false;
    }

    socket_ = std::move(candidate);
    lastReceiveSize_ = 0;

    if (logSink_)
    {
        logSink_("UdpListener::Open port=" + std::to_string(port));
    }
    return true;
}

void UdpListener::Close()
{
    if (socket_.IsValid() && logSink_)
    {
        logSink_("UdpListener::Close");
    }

    lastReceiveSize_ = 0;
    socket_.Reset();
}

int UdpListener::TickReceive()
{
    lastReceiveSize_ = 0;

    if (!socket_.IsValid())
    {
        return SOCKET_ERROR;
    }

    sockaddr_in sender{};
    int senderLength = sizeof(sender);
    const int received = ::recvfrom(
        socket_.Get(),
        reinterpret_cast<char*>(receiveBuffer_.data()),
        static_cast<int>(receiveBuffer_.size()),
        0,
        reinterpret_cast<sockaddr*>(&sender),
        &senderLength);

    if (received == SOCKET_ERROR)
    {
        const int error = ::WSAGetLastError();
        if (error != WSAEWOULDBLOCK && logSink_)
        {
            logSink_("UdpListener::TickReceive failed: " + std::to_string(error));
        }
        return (error == WSAEWOULDBLOCK) ? 0 : SOCKET_ERROR;
    }

    lastReceiveSize_ = received;
    if (logSink_)
    {
        logSink_("UdpListener::TickReceive bytes=" + std::to_string(received));
    }
    return received;
}

bool UdpListener::IsOpen() const
{
    return socket_.IsValid();
}

int UdpListener::GetLastReceiveSize() const
{
    return lastReceiveSize_;
}

void UdpListener::SetLogSink(LogSink sink)
{
    logSink_ = std::move(sink);
}

} // namespace TcpUdpSimulator
