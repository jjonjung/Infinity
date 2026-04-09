#pragma once

#include <cstddef>
#include <cstdint>

namespace TcpUdpSimulator
{

enum class EMessageType : std::uint16_t
{
    Unknown = 0,
    DeviceState = 1,
    Command = 2,
    CommandAck = 3
};

struct PacketHeader
{
    std::uint16_t MessageType = static_cast<std::uint16_t>(EMessageType::Unknown);
    std::uint16_t Version = 1;
    std::uint32_t BodySize = 0;
    std::uint32_t Sequence = 0;

    static constexpr std::size_t Size = sizeof(std::uint16_t) * 2 + sizeof(std::uint32_t) * 2;
};

} // namespace TcpUdpSimulator
