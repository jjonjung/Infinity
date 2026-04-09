#pragma once

#include <array>
#include <cstdint>

namespace TcpUdpSimulator
{

struct DeviceStatePacket
{
    std::array<char, 32> DeviceId {};
    float PositionX = 0.f;
    float PositionY = 0.f;
    float PositionZ = 0.f;
    float Speed = 0.f;
    std::uint32_t StatusCode = 0;
};

} // namespace TcpUdpSimulator
