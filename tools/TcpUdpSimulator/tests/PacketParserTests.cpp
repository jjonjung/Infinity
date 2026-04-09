#include "../include/Core/PacketParser.h"
#include "../include/Core/PacketSerializer.h"
#include "../include/Packet/CommandPacket.h"

#include <cassert>

using namespace TcpUdpSimulator;

int main()
{
    CommandPacket command {};
    command.CommandId = 101;

    auto bytes = PacketSerializer::Serialize(EMessageType::Command, 7, command);

    PacketParser parser;
    parser.Feed(bytes.data(), bytes.size());

    auto parsed = parser.TryParse();
    assert(parsed.has_value());
    assert(parsed->Header.Sequence == 7);
    assert(parsed->Header.BodySize == sizeof(CommandPacket));
    return 0;
}
