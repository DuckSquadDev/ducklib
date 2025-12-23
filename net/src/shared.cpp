#include <winsock2.h>
#include <string>
#include <WS2tcpip.h>
#include <cassert>
#include <format>

#include "ducklib/net/Shared.h"

#include <stdexcept>


namespace ducklib::net {
Address::Address(std::string_view address, uint16_t port) {
    this->port = port;
    IN_ADDR in_ddr;

    if (inet_pton(AF_INET, address.data(), &in_ddr) != 1)
        throw std::runtime_error("Failed to parse address");

    addr_v4_int = in_ddr.S_un.S_addr;
}

Address::Address(const sockaddr_in& sock_addr) {
    addr_v4_int = sock_addr.sin_addr.s_addr;
    port = ntohs(sock_addr.sin_port);
}

auto Address::get_address() const -> std::string {
    auto host_addr_int{ std::byteswap(addr_v4_int) };
    return std::format(
        "{}.{}.{}.{}",
        host_addr_int >> 24 & 0xFF,
        host_addr_int >> 16 & 0xFF,
        host_addr_int >> 8 & 0xFF,
        host_addr_int & 0xFF);
}


auto Address::as_sockaddr_in() const -> sockaddr_in {
    assert(addr_v4_int != 0);
    assert(port != 0);

    sockaddr_in sock_addr{};

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = addr_v4_int;

    return sock_addr;
}
}
