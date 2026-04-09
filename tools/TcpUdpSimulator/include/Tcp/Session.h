#pragma once

#include "../Core/PacketParser.h"
#include <cstdint>

namespace TcpUdpSimulator
{

class Session
{
public:
    explicit Session(std::uint32_t id)
        : id_(id)
    {
    }

    std::uint32_t GetId() const
    {
        return id_;
    }

    PacketParser& GetParser()
    {
        return parser_;
    }

private:
    std::uint32_t id_ = 0;
    PacketParser parser_;
};

} // namespace TcpUdpSimulator
