#include <gtest/gtest.h>
#include "net/shared.h"

using namespace ducklib::net;

TEST(address_tests, NewAddress_ShouldHave_DefaultPort0) {
    Address addr{ "127.0.0.1", 0 };
    ASSERT_EQ(0, addr.get_port());
    ASSERT_EQ("127.0.0.1", addr.get_address());
}
