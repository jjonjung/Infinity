#include "../include/Udp/UdpBroadcaster.h"
#include "../include/Udp/UdpListener.h"

#include <cassert>
#include <chrono>
#include <thread>

using namespace TcpUdpSimulator;

int main()
{
    UdpListener listener;
    UdpBroadcaster broadcaster;

    assert(listener.Open(7101));
    assert(broadcaster.Open("127.0.0.1", 7101));

    const char payload[] = "STATE";
    assert(broadcaster.Broadcast(payload, sizeof(payload)));

    int received = 0;
    for (int attempt = 0; attempt < 20 && received == 0; ++attempt)
    {
        received = listener.TickReceive();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(received == sizeof(payload));
    assert(listener.GetLastReceiveSize() == sizeof(payload));

    listener.Close();
    broadcaster.Close();
    return 0;
}
