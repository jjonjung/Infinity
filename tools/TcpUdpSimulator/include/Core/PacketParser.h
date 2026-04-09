#pragma once

#include "../Packet/PacketHeader.h"
#include "RingBuffer.h"
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

namespace TcpUdpSimulator
{

struct ParsedPacket
{
    PacketHeader Header;
    std::vector<std::uint8_t> Body;
};

class PacketParser
{
public:
    void Feed(const std::uint8_t* data, std::size_t length)
    {
        buffer_.Push(data, length);
    }

    std::optional<ParsedPacket> TryParse()
    {
        if (buffer_.Size() < PacketHeader::Size)
        {
            return std::nullopt;
        }

        std::uint8_t headerBytes[PacketHeader::Size] {};
        if (!buffer_.Peek(headerBytes, PacketHeader::Size))
        {
            return std::nullopt;
        }

        ParsedPacket packet;
        std::memcpy(&packet.Header.MessageType, headerBytes, sizeof(packet.Header.MessageType));
        std::memcpy(&packet.Header.Version, headerBytes + 2, sizeof(packet.Header.Version));
        std::memcpy(&packet.Header.BodySize, headerBytes + 4, sizeof(packet.Header.BodySize));
        std::memcpy(&packet.Header.Sequence, headerBytes + 8, sizeof(packet.Header.Sequence));

        const std::size_t totalSize = PacketHeader::Size + packet.Header.BodySize;
        if (buffer_.Size() < totalSize)
        {
            return std::nullopt;
        }

        buffer_.Discard(PacketHeader::Size);
        packet.Body.resize(packet.Header.BodySize);
        if (!packet.Body.empty())
        {
            buffer_.Pop(packet.Body.data(), packet.Body.size());
        }

        return packet;
    }

private:
    RingBuffer buffer_;
};

} // namespace TcpUdpSimulator
