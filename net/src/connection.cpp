#include <array>

#include "net/connection.h"

namespace ducklib::net {
MessageIdType Connection::send_reliable(std::span<std::byte> data, uint8_t channel, uint8_t priority) {
    return 0;
}

void Connection::send_queued() {
    auto& message = messages.top();
    uint16_t packet_size = 0;
    std::array<std::byte, MTU> packet;

    while (!messages.empty() && packet_size + message.size <= MTU) {
        messages.pop();
    }
}


}
