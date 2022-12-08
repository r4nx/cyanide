#include <cstdint>
#include <cyanide/patch.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Applying a patch", "[patches]")
{
    std::uint32_t target = 0;

    std::vector<cyanide::byte_t> patch_bytes{0x01, 0x00, 0x00, 0x00};

    cyanide::patch p{static_cast<void *>(&target), patch_bytes};

    REQUIRE(target == 1);
}
