#ifndef SHARED_H
#define SHARED_H

#include <string>
#include <cstdint>
#include <winsock.h>

namespace ducklib::net {
constexpr uint16_t MTU = 1200;

class Address {
public:
    Address() = default;
    Address(std::string_view address, uint16_t port);
    explicit Address(const sockaddr_in &sock_addr);

    [[nodiscard]] auto get_port() const -> uint16_t;
    [[nodiscard]] auto get_address() const -> std::string;
    [[nodiscard]] auto as_sockaddr_in() const -> sockaddr_in; // TODO: Consider moving these to NetClient

private:
    uint32_t addr_v4_int;
    uint16_t port;
};

inline auto Address::get_port() const -> uint16_t { return port; }
}
#endif // SHARED_H
