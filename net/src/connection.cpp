#include <array>

#include "net/connection.h"
#include "net/serialization.h"

namespace ducklib::net {
MessageIdType Connection::send_reliable(
    const std::byte* message_data,
    uint16_t message_bit_size,
    uint8_t type,
    bool ordered,
    uint8_t priority) {
    auto packet_id = channel_message_counter[type]++;
    auto byte_size = std::ceil(message_bit_size / 8.0f);
    PacketMessage message = {
        std::make_unique<std::byte[]>(byte_size),
        message_bit_size,
        packet_id,
        type,
        priority,
        ordered ? RELIABLE_ORDERED : RELIABLE
    };
    memcpy(message.data.get(), message_data, byte_size);
    message_send_queue.push(std::move(message));
    return packet_id;
}

void Connection::send_message_packet() {
    auto& message = message_send_queue.top();
    std::array<std::byte, MTU> packet;
    NetWriteStream writer(packet);
    PacketContents contents = {};

    while (!message_send_queue.empty() && writer.bits_left() >= message.data_bit_size) {
        message_send_queue.pop();

        // TODO: Write packet header

        // TODO: Write message header (ID, channel, size, data)
        // Should channel be used by different subsystems to identify type of message? I guess it basically is, and 256 different ones should be enough for anything

        // TODO: Track packet contents
    }
}

void Connection::serialize(NetWriteStream& writer, PacketMessage& message) {
    serialize_int(writer, message.delivery_mode, UNRELIABLE, RELIABLE_ORDERED);
    if (message.delivery_mode == RELIABLE_ORDERED) {
        serialize_int(writer, message.id, static_cast<PacketIdType>(0), static_cast<PacketIdType>(2));
    }
    serialize_int(writer, message.data_bit_size, static_cast<uint16_t>(0), MTU);
    serialize_data(writer, message.data.get(), message.data_bit_size);
}
}
