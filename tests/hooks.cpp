#include <catch2/catch_test_macros.hpp>
#include <cyanide/detour.hpp>

#include <functional> // std::bind_front
#include <utility>    // std::move

__declspec(noinline) int test_func_a(int x, int y)
{
    if (x == 0)
        return 0;

    x *= 2;
    y *= 3;

    return y / x;
}

TEST_CASE("Unhooked function test", "[hooks]")
{
    constexpr int x = 5, y = 7;
    constexpr int expected_result = 2;

    const int result = test_func_a(x, y);

    REQUIRE(result == expected_result);
}

// --------------------------------------------------------

TEST_CASE("Detour, lambda callback", "[hooks]")
{
    cyanide::Hooks::Detour detour{
        &test_func_a,
        [](decltype(&test_func_a) orig, int x, int y) -> int {
            return orig(x, y) + 1;
        }};

    detour.install();

    constexpr int x = 5, y = 7;
    constexpr int expected_result = 3;

    const int result = test_func_a(x, y);

    REQUIRE(result == expected_result);
}

// --------------------------------------------------------

class ReplacingHooker {
public:
    ReplacingHooker(int new_value) : new_value_{new_value} {}

    int callback(int x, int y)
    {
        return new_value_;
    }

private:
    int new_value_ = 0;
};

TEST_CASE("Detour, method callback")
{
    constexpr int expected_result = 10;

    ReplacingHooker                      hooker{expected_result};
    std::function<decltype(test_func_a)> callback{
        std::bind_front(&ReplacingHooker::callback, hooker)};

    cyanide::Hooks::Detour detour{&test_func_a, std::move(callback)};

    detour.install();

    constexpr int x = 5, y = 7;
    const int     result = test_func_a(x, y);

    REQUIRE(result == expected_result);
}
