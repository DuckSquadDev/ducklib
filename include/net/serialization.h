#ifndef DUCKLIB_SERIALIZATION_H
#define DUCKLIB_SERIALIZATION_H
#include <cassert>
#include <concepts>
#include <span>
#include <algorithm>
#include <cmath>
#include <bit>

namespace ducklib::net {
#define DL_NET_CHECK(expr) \
    do { \
        [[unlikely]] \
        if (!expr) { \
            return false; \
        } \
    } while (false)

template <typename T>
auto host_to_net(T value) -> T {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(value);
    } else {
        return value;
    }
}

using ScratchType = uint64_t;
constexpr uint8_t SCRATCH_SIZE_BITS = sizeof(ScratchType) * 8;

template <typename T>
auto net_to_host(T value) -> T {
    return host_to_net(value);
}

struct NetWriteStream {
    std::span<std::byte> buffer;
    uint32_t bits_written = 0; // Not including scratch_bits
    ScratchType scratch = 0;
    uint8_t scratch_bits = 0;

    NetWriteStream(std::span<std::byte> buffer) {
        this->buffer = buffer;
    }

    template <std::integral T>
    bool serialize_value(T value, uint8_t bits);
    bool serialize_data(std::byte* data, uint32_t data_bit_size);
    void align_to_byte();
    uint16_t bits_left() const;
    /**
     * @details When flushing, the next flush will happen to the following byte boundary.
     */
    bool flush_scratch(); // Requires scratch_bits to be up to date

    static constexpr bool can_write() { return true; }
    static constexpr bool can_read() { return false; }
};

#undef max

template <std::integral T>
bool NetWriteStream::serialize_value(T value, uint8_t bits) {
    assert(bits > 0);
    
    [[unlikely]]
    if (bits > bits_left()) {
        return false;
    }
    
    [[unlikely]]
    if (scratch_bits == SCRATCH_SIZE_BITS) {
        DL_NET_CHECK(flush_scratch());
    }

    auto remaining_scratch_bits = static_cast<uint8_t>(SCRATCH_SIZE_BITS - scratch_bits);
    auto mask = ~0ULL >> (SCRATCH_SIZE_BITS - bits);
    scratch |= (value & mask) << scratch_bits;
    auto spill_bits = static_cast<uint8_t>(bits < remaining_scratch_bits ? 0 : bits - remaining_scratch_bits);
    scratch_bits += bits - spill_bits;

    if (spill_bits > 0) {
        DL_NET_CHECK(flush_scratch());
        scratch = value >> remaining_scratch_bits;
        scratch_bits = spill_bits;
        return true;
    }

    return true;
}

struct NetReadStream {
    std::span<const std::byte> buffer;
    uint32_t bit_size = 0;
    uint32_t bits_read = 0;
    ScratchType scratch = 0;
    uint8_t scratch_bits = 0;          // Total bits loaded into scratch
    uint8_t scratch_bits_consumed = 0; // Bits consumed from scratch

    NetReadStream(const std::byte* data, uint32_t bit_size)
        : buffer(data, static_cast<size_t>(std::ceil(bit_size / 8.0)))
        , bit_size(bit_size) {
        read_scratch();
    }

    template <std::integral T>
    bool serialize_value(T& value, uint8_t bits);
    bool serialize_data(std::byte* data, uint16_t data_bit_size);
    void align_to_byte();
    uint16_t bits_left() const;
    bool read_scratch(); // Requires bits_read to be up to date but not scratch_bits_consumed

    static constexpr bool can_write() { return false; }
    static constexpr bool can_read() { return true; }
};

template <std::integral T>
bool NetReadStream::serialize_value(T& value, uint8_t bits) {
    assert(bits > 0);
    
    [[unlikely]]
    if (scratch_bits_consumed >= scratch_bits) {
        DL_NET_CHECK(read_scratch());
    }

    auto scratch_bits_remaining = static_cast<uint8_t>(scratch_bits - scratch_bits_consumed);
    auto mask = ~0ULL >> (SCRATCH_SIZE_BITS - bits);
    auto read_value = static_cast<T>((scratch >> scratch_bits_consumed) & mask);
    auto spill_bits = static_cast<uint8_t>(bits <= scratch_bits_remaining ? 0 : bits - scratch_bits_remaining);
    uint8_t read_bits_so_far = bits - spill_bits;
    scratch_bits_consumed += read_bits_so_far;

    if (spill_bits > 0) {
        DL_NET_CHECK(read_scratch());
        auto spill_mask = ~0ULL >> (SCRATCH_SIZE_BITS - spill_bits);
        read_value |= (scratch & spill_mask) << scratch_bits_remaining;
        scratch_bits_consumed = spill_bits;
    }

    value = read_value;
    return true;
}

template <typename StreamType, std::integral T>
bool serialize_int(StreamType& stream, T& value) {
    DL_NET_CHECK(stream.serialize_value(value, sizeof(T) * 8));
    return true;
}

template <typename StreamType, std::integral T>
bool serialize_int(StreamType& stream, T& value, T min, T max) {
    auto bits = static_cast<uint8_t>(std::bit_width(static_cast<uint64_t>(max - min)));
    T serialization_value = 0;

    if constexpr (stream.can_write()) {
        assert(min <= value && value <= max);
        T relative_value = value - min;
        serialization_value = relative_value;
    }

    DL_NET_CHECK(stream.serialize_value(serialization_value, bits));

    if constexpr (stream.can_read()) {
        value = serialization_value + min;
        assert(min <= value && value <= max);
    }

    return true;
}

template <typename StreamType>
bool serialize_data(StreamType& stream, std::byte* data, uint16_t data_bit_size) {
    DL_NET_CHECK(stream.serialize_data(data, data_bit_size));
    return true;
}
}

#endif //DUCKLIB_SERIALIZATION_H
