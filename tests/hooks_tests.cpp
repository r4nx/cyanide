#include <cyanide/memory/hook_backend_subhook.hpp>
#include <cyanide/memory/hook_frontend.hpp>

#include <catch2/catch_test_macros.hpp>

__declspec(noinline) int test_func_a(int x, int y)
{
    if (x == 0)
        return 0;

    x *= 2;
    y *= 3;

    return y / x;
}

TEST_CASE("Unhooked function")
{
    constexpr int x               = 3;
    constexpr int y               = 4;
    constexpr int expected_result = 2;

    const int actual_result = test_func_a(x, y);

    REQUIRE(actual_result == expected_result);
}

TEST_CASE("Detour")
{
    cyanide::memory::SubhookDetourBackend backend;
    cyanide::memory::DetourFrontend       frontend{
        &backend,
        &test_func_a,
        [](decltype(&test_func_a) orig, int x, int y) -> int {
            return orig(x, y) + 5;
        }};

    frontend.install();

    constexpr int x               = 3;
    constexpr int y               = 4;
    constexpr int expected_result = 7;

    const int actual_result = test_func_a(x, y);

    REQUIRE(actual_result == expected_result);
}
