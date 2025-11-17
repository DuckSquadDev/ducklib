#ifndef DUCKLIB_CONNECTION_H
#define DUCKLIB_CONNECTION_H
#include <cstdint>
#include <map>
#include <queue>
#include <span>
#include <unordered_map>
#include <vector>

#include "serialization.h"
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
    template <typename T>
    MessageIdType send_reliable(T& message, uint8_t type, bool ordered = false, uint8_t priority = MEDIUM_PRIORITY);
    MessageIdType send_reliable(
        const std::byte* message_data,
        uint16_t message_bit_size,
        uint8_t type,
        bool ordered = false,
        uint8_t priority = MEDIUM_PRIORITY);

    void send_message_packet();

private:
    static constexpr uint8_t UNRELIABLE = 0;
    static constexpr uint8_t RELIABLE = 1;
    static constexpr uint8_t RELIABLE_ORDERED = 2;
    
    struct PacketMessage {
        std::unique_ptr<std::byte[]> data;
        uint16_t data_bit_size;
        PacketIdType id;
        uint8_t type;
        uint8_t priority = MEDIUM_PRIORITY;
        uint8_t delivery_mode;

        bool operator <(const PacketMessage& other) const {
            return priority < other.priority;
        }
    };

    struct PacketContents {
        PacketIdType packet_id;
        std::vector<MessageIdType> messages;
    };

    static void serialize(NetWriteStream& writer, PacketMessage& message);
    
    Address remote_address;
    Socket socket;

    static constexpr auto LOW_PRIORITY = 0;
    static constexpr auto MEDIUM_PRIORITY = 1;
    static constexpr auto HIGH_PRIORITY = 2;

    uint32_t next_packet_id = 0;
    std::priority_queue<PacketMessage> message_send_queue;
    std::unordered_map<MessageIdType, PacketMessage> pending_messages;
    std::map<uint8_t, MessageIdType> channel_message_counter;

    std::unordered_map<PacketIdType, PacketContents> packet_message_map;
};

template <typename T>
MessageIdType Connection::send_reliable(T& message, uint8_t type, uint8_t priority) {}
}

#endif //DUCKLIB_CONNECTION_H
