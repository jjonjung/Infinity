#pragma once

#include "../Packet/PacketHeader.h"
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace TcpUdpSimulator
{

class PacketSerializer
{
public:
    template <typename TPacket>
    static std::vector<std::uint8_t> Serialize(EMessageType messageType, std::uint32_t sequence, const TPacket& packet)
    {
        static_assert(std::is_trivially_copyable_v<TPacket>, "Packet must be trivially copyable.");

        PacketHeader header;
        header.MessageType = static_cast<std::uint16_t>(messageType);
        header.BodySize = static_cast<std::uint32_t>(sizeof(TPacket));
        header.Sequence = sequence;

        std::vector<std::uint8_t> bytes(PacketHeader::Size + sizeof(TPacket));

        std::memcpy(bytes.data(), &header.MessageType, sizeof(header.MessageType));
        std::memcpy(bytes.data() + 2, &header.Version, sizeof(header.Version));
        std::memcpy(bytes.data() + 4, &header.BodySize, sizeof(header.BodySize));
        std::memcpy(bytes.data() + 8, &header.Sequence, sizeof(header.Sequence));
        std::memcpy(bytes.data() + PacketHeader::Size, &packet, sizeof(TPacket));

        return bytes;
    }
};

} // namespace TcpUdpSimulator
