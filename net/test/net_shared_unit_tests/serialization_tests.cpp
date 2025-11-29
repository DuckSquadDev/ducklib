#include <gtest/gtest.h>

#include "net/shared.h"
#include "net/serialization.h"

using namespace ducklib;

template <typename StreamT>
void small_serialization(StreamT& stream, int& first, int& second) {
    net::serialize_int(stream, first, 10, 200);
    net::serialize_int(stream, second, 60, 70);
}

TEST(serialization_tests, SmallSerialization_WithinScratchSize) {
    auto buffer_size = 128;
    auto buffer = std::make_unique<std::byte[]>(buffer_size);

    auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });
    auto first_write = 33;
    auto second_write = 66;
    small_serialization(writer, first_write, second_write);
    writer.flush_scratch();

    auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
    int first_read = 0;
    int second_read = 0;
    small_serialization(reader, first_read, second_read);

    ASSERT_EQ(first_read, first_write);
    ASSERT_EQ(second_read, second_write);
}

TEST(serialization_tests, BasicValueSerialization) {
    auto buffer_size = 128;
    auto buffer = std::make_unique<std::byte[]>(buffer_size);

    auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });
    uint8_t val8 = 0xAB;
    uint16_t val16 = 0x1234;
    uint32_t val32 = 0xDEADBEEF;

    ASSERT_TRUE(writer.serialize_value(val8, 8));
    ASSERT_TRUE(writer.serialize_value(val16, 16));
    ASSERT_TRUE(writer.serialize_value(val32, 32));
    writer.flush_scratch();

    auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
    uint8_t read8 = 0;
    uint16_t read16 = 0;
    uint32_t read32 = 0;

    ASSERT_TRUE(reader.serialize_value(read8, 8));
    ASSERT_TRUE(reader.serialize_value(read16, 16));
    ASSERT_TRUE(reader.serialize_value(read32, 32));

    ASSERT_EQ(read8, val8);
    ASSERT_EQ(read16, val16);
    ASSERT_EQ(read32, val32);
}

TEST(serialization_tests, ByteAlignment) {
    auto buffer_size = 128;
    auto buffer = std::make_unique<std::byte[]>(buffer_size);

    auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });
    uint8_t val = 0x7;  // 3 bits

    ASSERT_TRUE(writer.serialize_value(val, 3));
    ASSERT_EQ(writer.scratch_bits, 3);

    writer.align_to_byte();
    ASSERT_EQ(writer.scratch_bits, 8);

    ASSERT_TRUE(writer.serialize_value(val, 3));
    writer.flush_scratch();

    auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
    uint8_t read_val = 0;

    ASSERT_TRUE(reader.serialize_value(read_val, 3));
    ASSERT_EQ(read_val, val);
    ASSERT_EQ(reader.scratch_bits_consumed, 3);

    reader.align_to_byte();
    ASSERT_EQ(reader.scratch_bits_consumed, 8);
}

TEST(serialization_tests, BufferCapacity) {
    auto buffer_size = 16;
    auto buffer = std::make_unique<std::byte[]>(buffer_size);

    auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });

    ASSERT_EQ(writer.bits_left(), buffer_size * 8);

    uint64_t val = 0xFFFFFFFFFFFFFFFF;
    ASSERT_TRUE(writer.serialize_value(val, 64));
    ASSERT_EQ(writer.bits_left(), buffer_size * 8 - 64);

    ASSERT_TRUE(writer.serialize_value(val, 64));
    ASSERT_EQ(writer.bits_left(), 0);

    // Should fail when buffer is full
    ASSERT_FALSE(writer.serialize_value(val, 1));
}

TEST(serialization_tests, MultipleScratchFlushes) {
    auto buffer_size = 128;
    auto buffer = std::make_unique<std::byte[]>(buffer_size);

    auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });

    // Write multiple 64-bit values that will cause multiple flushes
    uint64_t val1 = 0x123456789ABCDEF0;
    uint64_t val2 = 0xFEDCBA9876543210;
    uint64_t val3 = 0xAAAABBBBCCCCDDDD;

    ASSERT_TRUE(writer.serialize_value(val1, 64));
    ASSERT_TRUE(writer.serialize_value(val2, 64));
    ASSERT_TRUE(writer.serialize_value(val3, 64));
    writer.flush_scratch();

    auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
    uint64_t read1 = 0, read2 = 0, read3 = 0;

    ASSERT_TRUE(reader.serialize_value(read1, 64));
    ASSERT_TRUE(reader.serialize_value(read2, 64));
    ASSERT_TRUE(reader.serialize_value(read3, 64));

    ASSERT_EQ(read1, val1);
    ASSERT_EQ(read2, val2);
    ASSERT_EQ(read3, val3);
}