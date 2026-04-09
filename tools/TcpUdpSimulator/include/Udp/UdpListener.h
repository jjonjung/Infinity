#pragma once

#include "../Core/LogSink.h"
#include "../Core/WinsockContext.h"

#include <cstddef>
#include <vector>

namespace TcpUdpSimulator
{

class UdpListener
{
public:
    ~UdpListener();

    bool Open(int port);
    void Close();
    int TickReceive();
    bool IsOpen() const;
    int GetLastReceiveSize() const;
    void SetLogSink(LogSink sink);

private:
    WinsockRuntime winsock_;
    SocketHandle socket_;
    std::vector<unsigned char> receiveBuffer_ = std::vector<unsigned char>(2048);
    int lastReceiveSize_ = 0;
    LogSink logSink_;
};

} // namespace TcpUdpSimulator
