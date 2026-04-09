# Packet Specification

## PacketHeader
- `MessageType`
- `Version`
- `BodySize`
- `Sequence`

## Messages
- `DeviceState`
- `Command`
- `CommandAck`

## Goals
- deterministic framing
- versionable payload structure
- transport-independent parsing
