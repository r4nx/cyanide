#define NOMINMAX

#include <cyanide/hook_impl_polyhook.hpp>
#include <cyanide/hook_wrapper.hpp>

#include <catch2/catch_test_macros.hpp>
#include <polyhook2/Detour/x86Detour.hpp>

#include <functional> // std::bind_front, std::function
#include <utility>    // std::move

__declspec(noinline) int test_func_a(int x, int y)
{
    if (x == 0)
        return 0;

    x *= 2;
    y *= 3;

    return y / x;
}

// Class for testing member function callback, used below
class Hooker {
public:
    Hooker(int replacing_value) : replacing_value_{replacing_value} {}

    int callback(int x, int y)
    {
        return replacing_value_;
    }

private:
    int replacing_value_ = 0;
};

TEST_CASE("Unhooked function", "[hooks]")
{
    constexpr int x               = 3;
    constexpr int y               = 4;
    constexpr int expected_result = 2;

    const int actual_result = test_func_a(x, y);

    REQUIRE(actual_result == expected_result);
}

TEST_CASE("Detour with lambda callback", "[hooks]")
{
    constexpr int x                      = 3;
    constexpr int y                      = 4;
    constexpr int expected_result        = 2;
    constexpr int expected_result_hooked = 7;

    {
        auto callback = [](decltype(&test_func_a) orig, int x, int y) -> int {
            return orig(x, y) + 5;
        };

        auto wrapper =
            cyanide::make_polyhook_x86(&test_func_a, std::move(callback));

        wrapper.install();

        const int actual_result = test_func_a(x, y);
        REQUIRE(actual_result == expected_result_hooked);
    }

    // Scope exited, the function must be unhooked at this point
    const int actual_result = test_func_a(x, y);
    REQUIRE(actual_result == expected_result);
}

TEST_CASE("Detour with member function callback", "[hooks]")
{
    constexpr int x                      = 3;
    constexpr int y                      = 4;
    constexpr int expected_result        = 2;
    constexpr int expected_result_hooked = 5;

    {
        Hooker hooker{5};

        std::function<decltype(test_func_a)> callback{
            std::bind_front(&Hooker::callback, &hooker)};

        auto wrapper =
            cyanide::make_polyhook_x86(&test_func_a, std::move(callback));

        wrapper.install();

        const int actual_result = test_func_a(x, y);
        REQUIRE(actual_result == expected_result_hooked);
    }

    const int actual_result = test_func_a(x, y);
    REQUIRE(actual_result == expected_result);
}
