#include <gtest/gtest.h>

#include "net/shared.h"
#include "net/serialization.h"

using namespace ducklib;

template <typename StreamT>
void small_serialization(StreamT& stream, int& first, int& second) {
    net::serialize(stream, first, 10, 200);
    net::serialize(stream, second, 60, 70);
}

TEST(serialization_tests, SmallSerialization_WithinScratchSize) {
    auto buffer_size = 128;
    auto buffer = std::make_unique<std::byte[]>(buffer_size);
    
    auto writer = net::NetWriteStream({ buffer.get(), static_cast<size_t>(buffer_size) });
    auto first_write = 33;
    auto second_write = 66;
    small_serialization(writer, first_write, second_write);
    writer.flush_scratch();

    auto reader = net::NetReadStream({ buffer.get(), static_cast<size_t>(buffer_size) });
    int first_read = 0;
    int second_read = 0;
    small_serialization(reader, first_read, second_read);

    ASSERT_EQ(first_read, first_write);
    ASSERT_EQ(second_read, second_write);
}