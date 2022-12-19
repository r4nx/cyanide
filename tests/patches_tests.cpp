#include <cyanide/patch.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

TEST_CASE("Applying a patch", "[patches]")
{
    std::uint32_t target_static  = 0;
    std::uint32_t target_dynamic = 0;

    auto dynamic_patch = cyanide::make_dynamic_patch(
        static_cast<void *>(&target_dynamic),
        0x05,
        0x00,
        0x00,
        0x00);

    auto static_patch = cyanide::make_static_patch(
        static_cast<void *>(&target_static),
        0x0A,
        0x00,
        0x00,
        0x00);

    REQUIRE(target_dynamic == 5);
    REQUIRE(target_static == 10);
}
