#include <winsock2.h>
#include <format>
#include <stdexcept>

#include "net/Net.h"
#include "core/logging/logger.h"

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

template <typename... Args>
void net_log_error(const char* text, Args... args)
{
    // static Logger logger;
    //
    // logger.log_error(text, args...);
}

template <typename... Args>
void net_fail(const std::string& text, Args... args)
{
    // net_log_error(text, args...);
    // exit(0);
}
}
