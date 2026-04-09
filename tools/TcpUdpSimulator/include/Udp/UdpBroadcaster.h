#pragma once

#include "../Core/LogSink.h"
#include "../Core/WinsockContext.h"
#include <cstddef>
#include <string>

namespace TcpUdpSimulator
{

class UdpBroadcaster
{
public:
    ~UdpBroadcaster();

    bool Open(const std::string& targetAddress, int port);
    void Close();
    bool Broadcast(const void* data, std::size_t size);
    bool IsOpen() const;
    void SetLogSink(LogSink sink);

private:
    WinsockRuntime winsock_;
    SocketHandle socket_;
    sockaddr_in target_{};
    LogSink logSink_;
};

} // namespace TcpUdpSimulator
