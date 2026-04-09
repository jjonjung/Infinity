#include "../../include/Udp/UdpBroadcaster.h"

#include <utility>

namespace TcpUdpSimulator
{

UdpBroadcaster::~UdpBroadcaster()
{
    Close();
}

bool UdpBroadcaster::Open(const std::string& targetAddress, int port)
{
    Close();

    SocketHandle candidate(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (!candidate.IsValid())
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("UdpBroadcaster::socket"));
        }
        return false;
    }

    BOOL enableBroadcast = TRUE;
    if (::setsockopt(candidate.Get(), SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&enableBroadcast), sizeof(enableBroadcast)) == SOCKET_ERROR)
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("UdpBroadcaster::setsockopt"));
        }
        return false;
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(static_cast<u_short>(port));
    const int convertResult = ::InetPtonA(AF_INET, targetAddress.c_str(), &target.sin_addr);
    if (convertResult != 1)
    {
        if (logSink_)
        {
            logSink_("UdpBroadcaster::Open invalid address: " + targetAddress);
        }
        return false;
    }

    socket_ = std::move(candidate);
    target_ = target;

    if (logSink_)
    {
        logSink_("UdpBroadcaster::Open -> " + targetAddress + ":" + std::to_string(port));
    }
    return true;
}

void UdpBroadcaster::Close()
{
    if (socket_.IsValid() && logSink_)
    {
        logSink_("UdpBroadcaster::Close");
    }

    socket_.Reset();
    target_ = sockaddr_in{};
}

bool UdpBroadcaster::Broadcast(const void* data, std::size_t size)
{
    if (!socket_.IsValid() || data == nullptr || size == 0)
    {
        return false;
    }

    const int sent = ::sendto(
        socket_.Get(),
        static_cast<const char*>(data),
        static_cast<int>(size),
        0,
        reinterpret_cast<const sockaddr*>(&target_),
        sizeof(target_));

    if (sent == SOCKET_ERROR)
    {
        if (logSink_)
        {
            logSink_(BuildSocketError("UdpBroadcaster::Broadcast"));
        }
        return false;
    }

    if (logSink_)
    {
        logSink_("UdpBroadcaster::Broadcast bytes=" + std::to_string(sent));
    }
    return true;
}

bool UdpBroadcaster::IsOpen() const
{
    return socket_.IsValid();
}

void UdpBroadcaster::SetLogSink(LogSink sink)
{
    logSink_ = std::move(sink);
}

} // namespace TcpUdpSimulator
