#include <cassert>
#include <bit>

#include "net/serialization.h"

#include <cmath>

namespace ducklib::net {
bool NetWriteStream::serialize_data(std::byte* data, uint32_t data_size_bits) {
    assert(data_size_bits > 0);

    if (data_size_bits > bits_left()) {
        return false;
    }
    
    auto remaining_scratch_bits = SCRATCH_SIZE_BITS - scratch_bits;

    // First part
    if (data_size_bits <= remaining_scratch_bits) {
        // Put all bits in a new scratch value so it can be shifted and put into the real scratch value
        auto full_byte_count = data_size_bits / 8;
        ScratchType temp_scratch = 0;
        memcpy(&temp_scratch, data, full_byte_count);
        auto tail_byte_mask = (1ULL << (data_size_bits - full_byte_count * 8)) - 1;
        temp_scratch |= (static_cast<uint8_t>(data[full_byte_count]) & tail_byte_mask) << (tail_byte_mask * 8);
        scratch |= temp_scratch << scratch_bits;
        scratch_bits += data_size_bits;
        return true;
    }
    
    auto head_tail_bits = -scratch_bits & 0x7;
    auto head_mask = (1ULL << head_tail_bits) - 1;
    scratch |= (static_cast<uint8_t>(*data) & head_mask) << scratch_bits;
    scratch_bits += head_tail_bits;
    flush_scratch();
    auto bits_to_write = data_size_bits - head_tail_bits;

    // Middle part
    auto whole_data_bytes_left = bits_to_write >> 3;
    auto data_bytes_written = 0;
    auto bytes_written = 0;
    auto data_bit_offset = head_tail_bits;
    if (data_bit_offset == 0) {
        memcpy(&buffer[bytes_written], &data[data_bytes_written], whole_data_bytes_left);
    } else {
        auto rev_data_offset = 8 - data_bit_offset;

        for (auto i = data_bytes_written; i < data_bytes_written + whole_data_bytes_left; ++i) {
            buffer[bytes_written++] = (data[i] << data_bit_offset) | (data[i+1] >> rev_data_offset);
        }
    }

    bits_to_write -= whole_data_bytes_left << 3;
    data_bytes_written += whole_data_bytes_left;

    // Last part
    if (bits_to_write > 0) {
        auto mask = (1ULL << bits_to_write) - 1;
        scratch = (static_cast<uint8_t>(data[data_bytes_written]) >> data_bit_offset) & mask;
        scratch_bits = bits_to_write;
    }

    bits_written += data_size_bits;
    return true;
}

void NetWriteStream::align_to_byte() {
    auto bit_offset_from_byte = scratch_bits & 0x7;
    scratch_bits += 8 - bit_offset_from_byte;
}

uint16_t NetWriteStream::bits_left() const {
    return buffer.size_bytes() * 8 - bits_written - scratch_bits;
}

bool NetWriteStream::flush_scratch() {
    auto bytes_written = std::ceil(bits_written / 8.0);
    auto scratch_bytes = std::ceil(scratch_bits / 8.0);

    [[unlikely]]
    if (buffer.size_bytes() < bytes_written + scratch_bytes) {
        return false;
    }

    [[likely]]
    if (scratch_bytes == sizeof(scratch)) {
        for (auto i = 0; i < sizeof(scratch); ++i) {
            buffer[bytes_written++] = static_cast<std::byte>((scratch >> (i * 8)) & 0xff);
        }
        bits_written += SCRATCH_SIZE_BITS;
    } else {
        for (auto i = 0; i < scratch_bytes; ++i) {
            buffer[bytes_written++] = static_cast<std::byte>((scratch >> (i * 8)) & 0xff);
        }
        bits_written += scratch_bytes * 8;
    }

    scratch_bits = 0;
    return true;
}

bool NetReadStream::serialize_data(std::byte* data, uint16_t data_bit_size) {
    auto byte_size = std::ceil(data_bit_size / 8.0f);

    // Just memcpy if byte-aligned write location
    if ((scratch_bits & 0x7) == 0) {
        auto first_value_size = scratch_bits;
        // serialize_value(
    }

    return true;
}
void NetReadStream::align_to_byte() {
    auto bit_offset_from_byte = scratch_bits & 0x7;
    if (bit_offset_from_byte != 0) {
        scratch_bits += 8 - bit_offset_from_byte;
    }
}

uint16_t NetReadStream::bits_left() const {
    return bit_size - bits_read - scratch_bits;
}

bool NetReadStream::read_scratch() {
    if (bits_read >= bit_size) {
        return false;
    }

    assert((bits_read & 0x7) == 0 && "Bits read should be a multiple of 8 when reading a new scratch");
    auto bytes_read = bits_read >> 3;
    auto bits_to_read = std::min(bit_size - bits_read, static_cast<uint32_t>(SCRATCH_SIZE_BITS));
    auto bytes_to_read = bits_to_read >> 3;
    scratch = 0;
    
    for (auto i = 0; i < bytes_to_read; ++i) {
        scratch |= static_cast<ScratchType>(static_cast<uint8_t>(buffer[bytes_read + i])) << (8 * i);
    }
    
    bytes_read += bytes_to_read;
    bits_to_read -= bytes_to_read * 8;
    scratch_bits = bytes_to_read * 8;
    
    if (bits_to_read) {
        auto tail_mask = (1ULL << bits_to_read) - 1;
        scratch |= (static_cast<uint8_t>(buffer[bytes_read]) & tail_mask) << scratch_bits;
        scratch_bits += bits_to_read;
    }
    
    return true;
}
}
