#include <cyanide/hooks/frontend.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iostream>

class Hooker : public cyanide::hooks::DetourBackendInterface {
public:
    void install(void *source, const void *destination) override
    {
        std::cout << "Installing hook at src: " << source
                  << ", dst: " << destination << std::endl;
    }

    void uninstall() override
    {
        std::cout << "Uninstalling the hook" << std::endl;
    }

    void *get_trampoline() override
    {
        std::cout << "Requested the trampoline" << std::endl;
        return nullptr;
    }
};

__declspec(noinline) int test_func_a(int x, int y)
{
    if (x == 0)
        return 0;

    x *= 2;
    y *= 3;

    return y / x;
}

TEST_CASE("Detour")
{
    Hooker backend;

    cyanide::hooks::DetourFrontend frontend{
        &backend,
        &test_func_a,
        [](decltype(&test_func_a) orig, int x, int y) -> int {
            return orig(x, y) + 1;
        }};

    frontend.install();
}
