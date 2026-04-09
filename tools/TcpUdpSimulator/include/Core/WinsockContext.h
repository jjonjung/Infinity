#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace TcpUdpSimulator
{

class WinsockRuntime
{
public:
    WinsockRuntime()
    {
        WSADATA data{};
        const int result = ::WSAStartup(MAKEWORD(2, 2), &data);
        if (result != 0)
        {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
        }
    }

    ~WinsockRuntime()
    {
        ::WSACleanup();
    }

    WinsockRuntime(const WinsockRuntime&) = delete;
    WinsockRuntime& operator=(const WinsockRuntime&) = delete;
    WinsockRuntime(WinsockRuntime&&) = delete;
    WinsockRuntime& operator=(WinsockRuntime&&) = delete;
};

class SocketHandle
{
public:
    SocketHandle() = default;
    explicit SocketHandle(SOCKET handle) noexcept
        : handle_(handle)
    {
    }

    ~SocketHandle()
    {
        Reset();
    }

    SocketHandle(const SocketHandle&) = delete;
    SocketHandle& operator=(const SocketHandle&) = delete;

    SocketHandle(SocketHandle&& other) noexcept
        : handle_(other.Release())
    {
    }

    SocketHandle& operator=(SocketHandle&& other) noexcept
    {
        if (this != &other)
        {
            Reset(other.Release());
        }
        return *this;
    }

    bool IsValid() const noexcept
    {
        return handle_ != INVALID_SOCKET;
    }

    SOCKET Get() const noexcept
    {
        return handle_;
    }

    void Reset(SOCKET newHandle = INVALID_SOCKET) noexcept
    {
        if (handle_ != INVALID_SOCKET)
        {
            ::closesocket(handle_);
        }
        handle_ = newHandle;
    }

    SOCKET Release() noexcept
    {
        const SOCKET released = handle_;
        handle_ = INVALID_SOCKET;
        return released;
    }

private:
    SOCKET handle_ = INVALID_SOCKET;
};

inline std::string BuildSocketError(const char* context)
{
    return std::string(context) + " failed: " + std::to_string(::WSAGetLastError());
}

inline bool SetNonBlocking(SOCKET socket, bool enabled)
{
    u_long mode = enabled ? 1UL : 0UL;
    return ::ioctlsocket(socket, FIONBIO, &mode) == 0;
}

} // namespace TcpUdpSimulator
