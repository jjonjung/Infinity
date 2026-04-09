# TcpUdpSimulator

## Goal
`TcpUdpSimulator` is a portfolio-oriented network test tool design for `Infinity`.

Its role is to validate gameplay-side network assumptions outside the engine runtime:
- TCP command and response testing
- UDP state broadcast testing
- packet structure validation
- timeout and reconnection handling

## Recommended Folder Structure
```text
Infinity/tools/TcpUdpSimulator
├── README.md
├── docs/
│   ├── architecture.md
│   ├── packet-spec.md
│   └── test-scenarios.md
├── include/
│   ├── Packet/
│   │   ├── PacketHeader.h
│   │   ├── DeviceStatePacket.h
│   │   └── CommandPacket.h
│   ├── Tcp/
│   │   ├── TcpClient.h
│   │   ├── TcpServer.h
│   │   └── Session.h
│   ├── Udp/
│   │   ├── UdpBroadcaster.h
│   │   └── UdpListener.h
│   ├── Core/
│   │   ├── RingBuffer.h
│   │   ├── PacketSerializer.h
│   │   ├── PacketParser.h
│   │   └── LogSink.h
│   └── App/
│       └── SimulatorApp.h
├── src/
│   ├── Packet/
│   ├── Tcp/
│   ├── Udp/
│   ├── Core/
│   └── App/
└── tests/
    ├── PacketParserTests.cpp
    ├── TcpLoopbackTests.cpp
    └── UdpBroadcastTests.cpp
```

## Why This Structure Works

### SRP
- `Packet`: payload schema and serialization contracts
- `Tcp`: reliable command channel
- `Udp`: low-latency state channel
- `Core`: shared data structures and parsing utilities
- `App`: executable wiring only

### OCP
New packet types can be added without rewriting the transport layer.

### Data Structure Strength
- `RingBuffer` for partial TCP receive handling
- explicit packet header for framing
- vector-based payload assembly
- packet parser isolated from UI or engine code

## Portfolio Positioning
This tool fits `Infinity` because it complements Unreal gameplay code with a standalone network validation layer. It shows:
- C++ systems design
- TCP/UDP understanding
- packet framing and parsing
- test-tool engineering
