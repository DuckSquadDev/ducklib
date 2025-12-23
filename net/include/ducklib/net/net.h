#ifndef NET_H
#define NET_H

#include <string>
#include <bit>

namespace ducklib::net {
void net_initialize();
void net_shutdown();

template <typename... Args>
void net_log_error(const char* text, Args... args);
template <typename... Args>
void net_fail(const std::string& text, Args... args);

template <typename T>
auto host_to_net(T value) -> T {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(value);
    } else {
        return value;
    }
}

template <typename T>
auto net_to_host(T value) -> T {
    return host_to_net(value);
}

template <typename... Args>
void net_log_error(const char* text, Args... args) {
    // static Logger logger;
    //
    // logger.log_error(text, args...);
}

template <typename... Args>
void net_fail(const std::string& text, Args... args) {
    // net_log_error(text, args...);
    // exit(0);
}
}

#endif // NET_H
