#include <cassert>
#include <bit>

#include "net/serialization.h"

namespace ducklib::net {
void NetWriteStream::align_to_byte() {
    auto bit_offset_from_byte = scratch_bits & 0x7;
    scratch_bits += 8 - bit_offset_from_byte;
}

bool NetWriteStream::flush_scratch() {
    if (buffer.size_bytes() < bytes_written + sizeof(scratch)) {
        return false;
    }
    
    auto net_value = host_to_net(scratch);
    memcpy(&buffer[bytes_written], &net_value, sizeof(net_value));
    bytes_written += sizeof(net_value);
    scratch_bits = 0;

    return true;
}

bool NetReadStream::read_scratch() {
    if (buffer.size_bytes() <= bytes_read) {
        return false;
    }

    net_to_host(*reinterpret_cast<ScratchType*>(&buffer[bytes_read]));

    return true;
}
}
