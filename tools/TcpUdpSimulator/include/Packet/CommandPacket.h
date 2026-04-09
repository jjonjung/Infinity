#pragma once

#include <array>
#include <cstdint>

namespace TcpUdpSimulator
{

struct CommandPacket
{
    std::uint32_t CommandId = 0;
    std::array<char, 32> TargetId {};
    std::array<char, 64> CommandText {};
};

} // namespace TcpUdpSimulator
