#include <winsock2.h>
#include <format>
#include <stdexcept>

#include "ducklib/net/Net.h"
#include "ducklib/core/logging/logger.h"

namespace ducklib::net
{
void net_initialize()
{
    WSAData data{};

    if (WSAStartup(MAKEWORD(2, 2), &data) == SOCKET_ERROR)
    {
        int error_code = WSAGetLastError();
        throw std::runtime_error(std::format("WinSock startup failed ({})", error_code));
    }
}

void net_shutdown()
{
    if (WSACleanup() == SOCKET_ERROR)
    {
        int errorCode = WSAGetLastError();
        throw std::runtime_error(std::format("WinSock cleanup failed ({})", errorCode));
    }
}
}
