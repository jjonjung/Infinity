cmake_minimum_required(VERSION 3.20)
project(TcpUdpSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(TcpUdpSimulator STATIC
    src/App/SimulatorApp.cpp
    src/Tcp/TcpClient.cpp
    src/Tcp/TcpServer.cpp
    src/Udp/UdpBroadcaster.cpp
    src/Udp/UdpListener.cpp
)

target_include_directories(TcpUdpSimulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TcpUdpSimulator PUBLIC ws2_32)

add_executable(PacketParserTests tests/PacketParserTests.cpp)
target_link_libraries(PacketParserTests PRIVATE TcpUdpSimulator)

add_executable(TcpLoopbackTests tests/TcpLoopbackTests.cpp)
target_link_libraries(TcpLoopbackTests PRIVATE TcpUdpSimulator)

add_executable(UdpBroadcastTests tests/UdpBroadcastTests.cpp)
target_link_libraries(UdpBroadcastTests PRIVATE TcpUdpSimulator)

enable_testing()
add_test(NAME PacketParserTests COMMAND PacketParserTests)
add_test(NAME TcpLoopbackTests COMMAND TcpLoopbackTests)
add_test(NAME UdpBroadcastTests COMMAND UdpBroadcastTests)
