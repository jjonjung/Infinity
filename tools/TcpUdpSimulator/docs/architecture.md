# TcpUdpSimulator Architecture

## Purpose
Standalone validation tool for command traffic and state broadcast traffic outside the Unreal runtime.

## Layered Structure
```text
[ App ]
   ->
[ TCP Layer ] + [ UDP Layer ]
   ->
[ Packet Parser / Serializer ]
   ->
[ RingBuffer / Session State ]
```

## Design Notes
- TCP handles reliable command-response flows.
- UDP handles low-latency state streaming.
- Packet framing is explicit through `PacketHeader`.
- `RingBuffer` handles partial TCP receives without mixing parsing concerns into transport code.
