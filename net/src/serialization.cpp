#include <cassert>

#include "ducklib/net/serialization.h"

namespace ducklib::net {
bool NetWriteStream::serialize_data(std::byte* data, uint32_t data_bit_size) {
    assert(data_bit_size > 0);

    if (data_bit_size > bits_left()) {
        return false;
    }
    
    uint8_t remaining_scratch_bits = SCRATCH_SIZE_BITS - scratch_bits;

    // First part
    if (data_bit_size <= remaining_scratch_bits) {
        // Put all bits in a new scratch value so it can be shifted and put into the real scratch value
        auto full_byte_count = data_bit_size >> 3;
        ScratchType temp_scratch = 0;
        memcpy(&temp_scratch, data, full_byte_count);
        auto tail_byte_bits = (data_bit_size - full_byte_count * 8);
        auto tail_byte_mask = tail_byte_bits > 0 ? ~0ULL >> (SCRATCH_SIZE_BITS - tail_byte_bits) : 0;
        temp_scratch |= (static_cast<uint8_t>(data[full_byte_count]) & tail_byte_mask) << (full_byte_count * 8);
        scratch |= temp_scratch << scratch_bits;
        scratch_bits += data_bit_size;
        return true;
    }
    
    auto head_tail_bits = -scratch_bits & 0x7;
    auto head_mask = head_tail_bits > 0 ? ~0ULL >> (SCRATCH_SIZE_BITS - head_tail_bits) : 0;
    scratch |= (static_cast<uint8_t>(*data) & head_mask) << scratch_bits;
    scratch_bits += head_tail_bits;
    flush_scratch();
    auto bits_to_write = data_bit_size - head_tail_bits;

    // Middle part
    const auto whole_data_bytes_left = bits_to_write >> 3;
    auto data_bytes_written = 0U;
    auto bytes_written = bits_written >> 3;
    const auto data_bit_offset = head_tail_bits;
    if (data_bit_offset == 0) {
        memcpy(&buffer[bytes_written], &data[data_bytes_written], whole_data_bytes_left);
    } else {
        const auto rev_data_offset = 8 - data_bit_offset;

        for (auto i = data_bytes_written; i < data_bytes_written + whole_data_bytes_left; ++i) {
            buffer[bytes_written++] = (data[i] >> data_bit_offset) | (data[i+1] << rev_data_offset);
        }
    }

    bits_to_write -= whole_data_bytes_left << 3;
    data_bytes_written += whole_data_bytes_left;

    // Last part
    if (bits_to_write > 0) {
        auto mask = ~0ULL >> (SCRATCH_SIZE_BITS - bits_to_write);
        scratch = (static_cast<uint8_t>(data[data_bytes_written]) >> data_bit_offset) & mask;
        scratch_bits = (uint8_t)bits_to_write;
    }

    bits_written += data_bit_size;
    return true;
}

void NetWriteStream::align_to_byte() {
    auto bit_offset_from_byte = scratch_bits & 0x7;
    scratch_bits += (8 - bit_offset_from_byte) & 0x7;
}

uint16_t NetWriteStream::bits_left() const {
    return uint16_t(buffer.size_bytes() * 8 - bits_written - scratch_bits);
}

bool NetWriteStream::flush_scratch() {
    auto bytes_written = (bits_written + 7) >> 3;
    auto scratch_bytes = (scratch_bits + 7) >> 3;

    [[unlikely]]
    if (buffer.size_bytes() < bytes_written + scratch_bytes) {
        return false;
    }

    [[likely]]
    if (scratch_bytes == sizeof(scratch)) {
        for (auto i = 0U; i < sizeof(scratch); ++i) {
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
    scratch = 0;
    return true;
}

/// head, body, and tail terms are used for the first, middle, and last part of the functionality
bool NetReadStream::serialize_data(std::byte* data, uint16_t data_bit_size) {
    assert(data_bit_size > 0);
    
    if (data_bit_size > bits_left()) {
        return false;
    }
    
    // First part (to consume scratch and reach a byte boundary in the read buffer (data))
    auto head_scratch_bits = std::min(data_bit_size, static_cast<uint16_t>(scratch_bits - scratch_bits_consumed));
    auto head_scratch_byte_count = head_scratch_bits >> 3;

    for (auto i = 0; i < head_scratch_byte_count; ++i) {
        data[i] = static_cast<std::byte>((scratch >> (scratch_bits_consumed + i * 8)) & 0xff);
    }
    
    auto head_byte_bits_copied = head_scratch_byte_count * 8;
    scratch_bits_consumed += head_byte_bits_copied;
    auto trailing_scratch_bits = head_scratch_bits - head_byte_bits_copied;
    auto trailing_scratch_mask = trailing_scratch_bits > 0 ? ~0ULL >> (SCRATCH_SIZE_BITS - trailing_scratch_bits) : 0;
    data[head_scratch_byte_count] = static_cast<std::byte>((scratch >> scratch_bits_consumed) & trailing_scratch_mask);
    scratch_bits_consumed += trailing_scratch_bits;
    
    auto head_bits_written = head_byte_bits_copied + trailing_scratch_bits;
    auto buffer_offset_bits = std::min((8 - (head_bits_written & 0x7)) & 0x7, data_bit_size - head_bits_written); // TODO: Reconsider min???
    auto align_mask = static_cast<std::byte>(~0U) >> (8 - buffer_offset_bits);
    data[head_scratch_byte_count] |= (buffer[bits_read >> 3] & align_mask) << (8 - buffer_offset_bits);
    bits_read += buffer_offset_bits;
    
    auto bits_copied = head_byte_bits_copied + trailing_scratch_bits + buffer_offset_bits;
    
    // Middle part (just loop bytes)
    auto middle_bits = data_bit_size - bits_copied;
    auto middle_bytes = middle_bits >> 3;
    auto start_dest_byte = bits_copied >> 3;
    auto start_src_byte = bits_read >> 3;
    
    if (buffer_offset_bits == 0) {
        memcpy(&data[start_dest_byte], &buffer[start_src_byte], middle_bytes);
    } else {
        auto low_offset = buffer_offset_bits;
        auto high_offset = 8 - buffer_offset_bits;
        auto low_mask = static_cast<std::byte>(~0U) >> (8 - high_offset);
        auto high_mask = static_cast<std::byte>(~0U) >> (8 - low_offset);

        for (auto i = 0; i < middle_bytes; ++i) {
            data[start_dest_byte + i] = ((buffer[start_src_byte + i] >> low_offset) & low_mask)
                | ((buffer[start_src_byte + i + 1] & high_mask) << high_offset);
        }
    }
    
    bits_read += middle_bits;
    bits_copied += middle_bits;
    
    // Last part (read leftovers)
    auto last_bits = data_bit_size - bits_copied;
    auto last_mask = static_cast<std::byte>(~0U >> (8 - last_bits));
    data[bits_read >> 3] |= (buffer[bits_copied >> 3] >> (8 - last_bits)) & last_mask;
    bits_read += last_bits;

    return true;
}

void NetReadStream::align_to_byte() {
    auto bit_offset_from_byte = scratch_bits_consumed & 0x7;
    scratch_bits_consumed += (8 - bit_offset_from_byte) & 0x7;
}

uint16_t NetReadStream::bits_left() const {
    return uint16_t((bit_size - bits_read) + (scratch_bits - scratch_bits_consumed));
}

bool NetReadStream::read_scratch() {
    if (bits_read >= bit_size) {
        return false;
    }

    assert((bits_read & 0x7) == 0 && "Bits read should be a multiple of 8 when reading a new scratch");
    auto total_bytes_read = bits_read >> 3;
    auto bits_to_read = std::min(bit_size - bits_read, static_cast<uint32_t>(SCRATCH_SIZE_BITS));
    auto bytes_to_read = bits_to_read >> 3;
    scratch = 0;
    
    for (auto i = 0U; i < bytes_to_read; ++i) {
        scratch |= static_cast<ScratchType>(static_cast<uint8_t>(buffer[total_bytes_read + i])) << (8 * i);
    }
    
    total_bytes_read += bytes_to_read;
    auto trailing_bits = bits_to_read - bytes_to_read * 8;
    
    if (trailing_bits) {
        auto tail_mask = ~0ULL >> (SCRATCH_SIZE_BITS - trailing_bits);
        scratch |= (static_cast<uint8_t>(buffer[total_bytes_read]) & tail_mask) << (bytes_to_read * 8U);
    }
    
    bits_read += bits_to_read;
    scratch_bits = (uint8_t)bits_to_read;
    scratch_bits_consumed = 0;

    return true;
}
}
