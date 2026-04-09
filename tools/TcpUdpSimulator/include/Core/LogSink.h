#pragma once

#include <functional>
#include <string>

namespace TcpUdpSimulator
{

using LogSink = std::function<void(const std::string&)>;

} // namespace TcpUdpSimulator
