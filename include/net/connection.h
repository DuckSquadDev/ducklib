#ifndef DUCKLIB_CONNECTION_H
#define DUCKLIB_CONNECTION_H
#include <cstdint>
#include <map>
#include <queue>
#include <span>

#include "Socket.h"

namespace ducklib::net {
using PacketIdType = uint32_t;
using MessageIdType = uint32_t;
using AckTrailType = uint32_t;
constexpr auto NUM_ACK_BITS = sizeof(AckTrailType) * 8;
constexpr auto MAX_TRACKED_MESSAGES = 256;
constexpr auto DEFAULT_CHANNEL = 0;

class Connection {
public:
    MessageIdType send_reliable(std::span<std::byte> data, uint8_t channel = DEFAULT_CHANNEL, uint8_t priority = MEDIUM_PRIORITY);

    void send_queued();

private:
    Address remote_address;
    Socket socket;

    static constexpr auto LOW_PRIORITY = 0;
    static constexpr auto MEDIUM_PRIORITY = 1;
    static constexpr auto HIGH_PRIORITY = 2;

    struct TrackedMessage {
        std::unique_ptr<std::byte> data;
        PacketIdType id;
        uint16_t size;
        uint8_t type;
        uint8_t priority = MEDIUM_PRIORITY;

        bool operator < (const TrackedMessage& other) const {
            return priority < other.priority;
        }
    };
    uint32_t next_packet_id = 0;
    std::priority_queue<TrackedMessage> messages;
    std::map<uint8_t, MessageIdType> channel_message_counter;
};
}

#endif //DUCKLIB_CONNECTION_H