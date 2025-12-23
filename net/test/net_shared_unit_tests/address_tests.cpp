#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN  // Only define this in one of the source files per project

#include "third_party/doctest.h"
#include "ducklib/net/shared.h"

using namespace ducklib::net;

TEST_CASE("NewAddress_ShouldHave_DefaultPort0") {
    Address addr{ "127.0.0.1", 0 };
    REQUIRE_EQ(0, addr.get_port());
    REQUIRE_EQ("127.0.0.1", addr.get_address());
}
