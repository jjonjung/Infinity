#pragma once

#include "../Core/LogSink.h"
#include "../Core/WinsockContext.h"
#include <cstddef>
#include <string>

namespace TcpUdpSimulator
{

class TcpClient
{
public:
    ~TcpClient();

    bool Connect(const std::string& host, int port);
    void Disconnect();
    bool Send(const void* data, std::size_t size);
    int Receive(void* buffer, std::size_t size);
    bool IsConnected() const;
    void SetLogSink(LogSink sink);

private:
    WinsockRuntime winsock_;
    SocketHandle socket_;
    LogSink logSink_;
};

} // namespace TcpUdpSimulator
