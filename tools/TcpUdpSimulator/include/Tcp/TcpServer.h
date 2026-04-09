#pragma once

#include "../Core/LogSink.h"
#include "../Core/WinsockContext.h"

#include <vector>

namespace TcpUdpSimulator
{

class TcpServer
{
public:
    ~TcpServer();

    bool Start(int port);
    void Stop();
    void TickAccept();
    void TickReceive();
    bool HasClient() const;
    int GetLastReceiveSize() const;
    void SetLogSink(LogSink sink);

private:
    WinsockRuntime winsock_;
    SocketHandle listenSocket_;
    SocketHandle clientSocket_;
    int lastReceiveSize_ = 0;
    LogSink logSink_;
};

} // namespace TcpUdpSimulator
