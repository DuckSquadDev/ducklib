#ifndef DUCKLIB_SERIALIZATION_H
#define DUCKLIB_SERIALIZATION_H
#include <concepts>
#include <span>
#include <algorithm>

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

template <typename T>
auto net_to_host(T value) -> T {
    return host_to_net(value);
}

struct NetWriteStream {
    std::span<std::byte> buffer;
    uint32_t bytes_written = 0;
    ScratchType scratch = 0;
    uint8_t scratch_bits = 0;

    NetWriteStream(std::span<std::byte> buffer) {
        assert((buffer.size_bytes() & (sizeof(scratch) - 1)) == 0);
        assert(buffer.size_bytes() >= sizeof(scratch));
        this->buffer = buffer;
    }

    template <std::integral T>
    bool serialize_bits(T value, uint8_t bits);
    void align_to_byte();
    bool flush_scratch(); // TODO: Consider how to avoid large zero tail for using uint64_t (bytes_written will increment 8 on last flush)

    static bool can_write() { return true; }
    static bool can_read() { return false; }
};

#undef max

template <std::integral T>
bool NetWriteStream::serialize_bits(T value, uint8_t bits) {
    [[unlikely]]
    if (scratch_bits == sizeof(scratch) * 8) {
        DL_NET_CHECK(flush_scratch());
    }

    auto remaining_scratch_bits = static_cast<uint8_t>(sizeof(scratch) * 8 - scratch_bits);
    auto mask = (1ULL << bits) - 1;
    scratch |= (value & mask) << scratch_bits;
    auto spill_bits = static_cast<uint8_t>(bits < remaining_scratch_bits ? 0 : bits - remaining_scratch_bits);

    if (spill_bits > 0) {
        DL_NET_CHECK(flush_scratch());
        scratch = value >> spill_bits;
        scratch_bits = spill_bits;
        return true;
    }

    scratch_bits += bits;
    return true;
}

struct NetReadStream {
    std::span<std::byte> buffer;
    uint32_t bytes_read = 0;
    ScratchType scratch = 0;
    uint8_t scratch_bits = 0;

    NetReadStream(std::span<std::byte> buffer) {
        assert((buffer.size_bytes() & (sizeof(scratch) - 1)) == 0);
        assert(buffer.size_bytes() >= sizeof(scratch));
        this->buffer = buffer;
        scratch = net_to_host(*reinterpret_cast<ScratchType*>(buffer.data()));
    }

    template <std::integral T>
    bool serialize_bits(T& value, uint8_t bits);
    void align_to_byte();
    bool read_scratch();

    static bool can_write() { return false; }
    static bool can_read() { return true; }
};

template <std::integral T>
bool NetReadStream::serialize_bits(T& value, uint8_t bits) {
    [[unlikely]]
    if (scratch_bits == sizeof(scratch) * 8) {
        DL_NET_CHECK(read_scratch());
    }
    
    auto remaining_scratch_bits = static_cast<uint8_t>(sizeof(scratch) * 8 - scratch_bits);
    auto mask = (1ULL << bits) - 1;
    auto read_value = static_cast<T>((scratch >> scratch_bits) & mask);
    auto spill_bits = static_cast<uint8_t>(bits < remaining_scratch_bits ? 0 : bits - remaining_scratch_bits);

    if (spill_bits > 0) {
        DL_NET_CHECK(read_scratch());
        auto spill_mask = (1ULL << spill_bits) - 1;
        read_value |= (scratch & spill_mask) << remaining_scratch_bits;
        scratch_bits = spill_bits;
    } else {
        scratch_bits += bits;
    }

    value = read_value;
    return true;
}

template <typename StreamType, std::integral T>
bool serialize(StreamType& stream, T& value, T min, T max) {
    auto bits = static_cast<uint8_t>(std::bit_width(static_cast<uint64_t>(max - min)));
    T serialization_value = 0;
    
    if (stream.can_write()) {
        assert(min <= value && value <= max);
        T relative_value = value - min;
        serialization_value = relative_value;
    }
    
    DL_NET_CHECK(stream.serialize_bits(serialization_value, bits));

    if (stream.can_read()) {
        value = serialization_value + min;
        assert(min <= value && value <= max);
    }
    
    return true;
}
}

#endif //DUCKLIB_SERIALIZATION_H