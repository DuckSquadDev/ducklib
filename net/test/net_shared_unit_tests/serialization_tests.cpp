#include <memory>
#include "third_party/doctest.h"
#include "ducklib/net/shared.h"
#include "ducklib/net/serialization.h"

using namespace ducklib;

template <typename StreamT>
void small_serialization(StreamT& stream, int& first, int& second) {
    net::serialize_int(stream, first, 10, 200);
    net::serialize_int(stream, second, 60, 70);
}

TEST_SUITE("serialization") {
    TEST_CASE("SmallSerialization_WithinScratchSize") {
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

        REQUIRE_EQ(first_read, first_write);
        REQUIRE_EQ(second_read, second_write);
    }

    TEST_CASE("BasicValueSerialization") {
        auto buffer_size = 128;
        auto buffer = std::make_unique<std::byte[]>(buffer_size);

        auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });
        uint8_t val8 = 0xAB;
        uint16_t val16 = 0x1234;
        uint32_t val32 = 0xDEADBEEF;

        REQUIRE(writer.serialize_value(val8, 8));
        REQUIRE(writer.serialize_value(val16, 16));
        REQUIRE(writer.serialize_value(val32, 32));
        writer.flush_scratch();

        auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
        uint8_t read8 = 0;
        uint16_t read16 = 0;
        uint32_t read32 = 0;

        REQUIRE(reader.serialize_value(read8, 8));
        REQUIRE(reader.serialize_value(read16, 16));
        REQUIRE(reader.serialize_value(read32, 32));

        REQUIRE_EQ(read8, val8);
        REQUIRE_EQ(read16, val16);
        REQUIRE_EQ(read32, val32);
    }

    TEST_CASE("ByteAlignment") {
        auto buffer_size = 128;
        auto buffer = std::make_unique<std::byte[]>(buffer_size);

        auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });
        uint8_t val = 0x7;  // 3 bits

        REQUIRE(writer.serialize_value(val, 3));
        REQUIRE_EQ(writer.scratch_bits, 3);

        writer.align_to_byte();
        REQUIRE_EQ(writer.scratch_bits, 8);

        REQUIRE(writer.serialize_value(val, 3));
        writer.flush_scratch();

        auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
        uint8_t read_val = 0;

        REQUIRE(reader.serialize_value(read_val, 3));
        REQUIRE_EQ(read_val, val);
        REQUIRE_EQ(reader.scratch_bits_consumed, 3);

        reader.align_to_byte();
        REQUIRE_EQ(reader.scratch_bits_consumed, 8);
    }

    TEST_CASE("BufferCapacity") {
        auto buffer_size = 16;
        auto buffer = std::make_unique<std::byte[]>(buffer_size);

        auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });

        REQUIRE_EQ(writer.bits_left(), buffer_size * 8);

        uint64_t val = 0xFFFFFFFFFFFFFFFF;
        REQUIRE(writer.serialize_value(val, 64));
        REQUIRE_EQ(writer.bits_left(), buffer_size * 8 - 64);

        REQUIRE(writer.serialize_value(val, 64));
        REQUIRE_EQ(writer.bits_left(), 0);

        // Should fail when buffer is full
        REQUIRE_FALSE(writer.serialize_value(val, 1));
    }

    TEST_CASE("MultipleScratchFlushes") {
        auto buffer_size = 128;
        auto buffer = std::make_unique<std::byte[]>(buffer_size);

        auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });

        // Write multiple 64-bit values that will cause multiple flushes
        uint64_t val1 = 0x123456789ABCDEF0;
        uint64_t val2 = 0xFEDCBA9876543210;
        uint64_t val3 = 0xAAAABBBBCCCCDDDD;

        REQUIRE(writer.serialize_value(val1, 64));
        REQUIRE(writer.serialize_value(val2, 64));
        REQUIRE(writer.serialize_value(val3, 64));
        writer.flush_scratch();

        auto reader = net::NetReadStream({ buffer.get(), buffer_size * 8U });
        uint64_t read1 = 0, read2 = 0, read3 = 0;

        REQUIRE(reader.serialize_value(read1, 64));
        REQUIRE(reader.serialize_value(read2, 64));
        REQUIRE(reader.serialize_value(read3, 64));

        REQUIRE_EQ(read1, val1);
        REQUIRE_EQ(read2, val2);
        REQUIRE_EQ(read3, val3);
    }

    TEST_CASE("SerializeDataWithOffset") {
        constexpr auto buffer_size = 128;
        constexpr auto main_bit_size = 243;
        auto source_buffer = std::make_unique<std::byte[]>(buffer_size);
        auto value_writer = net::NetWriteStream({ source_buffer.get(), static_cast<size_t>(buffer_size) });

        // Write values
        auto a = 12345U; auto b = 67890U; auto c = 11U; auto d = 250000000U; auto e = 111111U; auto f = 0U;
        auto g = 22222U; auto h = 65355ULL; auto i = 3U;
        REQUIRE(value_writer.serialize_value(a, 16));
        REQUIRE(value_writer.serialize_value(b, 32));
        REQUIRE(value_writer.serialize_value(c, 5));
        REQUIRE(value_writer.serialize_value(d, 32));
        REQUIRE(value_writer.serialize_value(e, 23));
        REQUIRE(value_writer.serialize_value(f, 29));
        REQUIRE(value_writer.serialize_value(g, 15));
        REQUIRE(value_writer.serialize_value(h, 64));
        REQUIRE(value_writer.serialize_value(i, 27));
        REQUIRE(value_writer.flush_scratch());
    
        // Block copy part
        auto block_source_buffer = std::make_unique<std::byte[]>(buffer_size);
        auto block_writer = net::NetWriteStream({ block_source_buffer.get(), buffer_size });

        auto x = 4U;    
        REQUIRE(block_writer.serialize_value(x, 3));
        REQUIRE(block_writer.serialize_data(value_writer.buffer.data(), main_bit_size));
    
        auto block_dest_buffer = std::make_unique<std::byte[]>(buffer_size);
        auto block_reader = net::NetReadStream({ block_writer.buffer.data(), main_bit_size + 3 });

        uint32_t rx;
        REQUIRE(block_reader.serialize_value(rx, 3));
        REQUIRE(block_reader.serialize_data(block_dest_buffer.get(), main_bit_size));
    
        // Read back all values
        auto value_reader = net::NetReadStream(block_dest_buffer.get(), main_bit_size);
        uint16_t ra; uint32_t rb; uint8_t rc; uint32_t rd; uint32_t re; uint32_t rf;
        uint16_t rg; uint64_t rh; uint32_t ri;
    
        REQUIRE(value_reader.serialize_value(ra, 16));
        REQUIRE(value_reader.serialize_value(rb, 32));
        REQUIRE(value_reader.serialize_value(rc, 5));
        REQUIRE(value_reader.serialize_value(rd, 32));
        REQUIRE(value_reader.serialize_value(re, 23));
        REQUIRE(value_reader.serialize_value(rf, 29));
        REQUIRE(value_reader.serialize_value(rg, 15));
        REQUIRE(value_reader.serialize_value(rh, 64));
        REQUIRE(value_reader.serialize_value(ri, 27));
    
        REQUIRE_EQ(a, ra);
        REQUIRE_EQ(b, rb);
        REQUIRE_EQ(c, rc);
        REQUIRE_EQ(d, rd);
        REQUIRE_EQ(e, re);
        REQUIRE_EQ(f, rf);
        REQUIRE_EQ(g, rg);
        REQUIRE_EQ(h, rh);
        REQUIRE_EQ(i, ri);
    }

    TEST_CASE("SerializeDataFullScratch") {
        constexpr auto buffer_size = 128;
        auto source_buffer = std::make_unique<std::byte[]>(buffer_size);
        auto writer = net::NetWriteStream({ source_buffer.get(), static_cast<size_t>(buffer_size) });
        uint64_t data[] = { 0x1234567890abcdefULL, 0xcafebabedeadbeefULL };
        uint64_t a = 0;
        uint16_t data_size = sizeof(data) * 8;
    
        REQUIRE(writer.serialize_value(a, 64));
        REQUIRE(writer.serialize_data(reinterpret_cast<std::byte*>(data), data_size));

        auto block_buffer = std::make_unique<std::byte[]>(buffer_size);
        auto reader = net::NetReadStream({ source_buffer.get(), static_cast<uint16_t>(data_size + 64) });
    
        uint64_t ra;
        REQUIRE(reader.serialize_value(ra, 64));
        REQUIRE(reader.serialize_data(block_buffer.get(), data_size));
        REQUIRE_EQ(a, ra);
        REQUIRE_EQ(reinterpret_cast<uint64_t*>(block_buffer.get())[0], data[0]);
        REQUIRE_EQ(reinterpret_cast<uint64_t*>(block_buffer.get())[1], data[1]);
    }
}