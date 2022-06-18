#define NOMINMAX

#include <cyanide/memory/hook_backend_subhook.hpp>
#include <cyanide/memory/hook_frontend.hpp>
#include <cyanide/memory/signature_scanner_win.hpp>

#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("Unhooked function", "[memory][hooks]")
{
    constexpr int x               = 3;
    constexpr int y               = 4;
    constexpr int expected_result = 2;

    const int actual_result = test_func_a(x, y);

    REQUIRE(actual_result == expected_result);
}

TEST_CASE("Detour with lambda callback", "[memory][hooks]")
{
    constexpr int x                      = 3;
    constexpr int y                      = 4;
    constexpr int expected_result        = 2;
    constexpr int expected_result_hooked = 7;

    {
        cyanide::memory::SubhookDetourBackend backend;
        cyanide::memory::DetourFrontend       frontend{
            &backend,
            &test_func_a,
            [](decltype(&test_func_a) orig, int x, int y) -> int {
                return orig(x, y) + 5;
            }};

        frontend.install();

        const int actual_result = test_func_a(x, y);
        REQUIRE(actual_result == expected_result_hooked);
    }

    // Scope exited, the function must be unhooked at this point
    const int actual_result = test_func_a(x, y);
    REQUIRE(actual_result == expected_result);
}

TEST_CASE("Detour with member function callback", "[memory][hooks]")
{
    constexpr int x                      = 3;
    constexpr int y                      = 4;
    constexpr int expected_result        = 2;
    constexpr int expected_result_hooked = 5;

    {
        Hooker hooker{5};

        std::function<decltype(test_func_a)> callback{
            std::bind_front(&Hooker::callback, &hooker)};

        cyanide::memory::SubhookDetourBackend backend;
        cyanide::memory::DetourFrontend       frontend{
            &backend,
            &test_func_a,
            std::move(callback)};

        frontend.install();

        const int actual_result = test_func_a(x, y);
        REQUIRE(actual_result == expected_result_hooked);
    }

    const int actual_result = test_func_a(x, y);
    REQUIRE(actual_result == expected_result);
}
