#include "detour.hpp"

#include <cstdio>

__declspec(noinline) int __stdcall foo(int x, int y)
{
    return x + y;
}

int main()
{
    cyanide::Hooks::Detour hook{&foo, [](decltype(&foo) orig, int x, int y) {
                                    return x * y;
                                }};
    hook.install();

    printf("%d", foo(3, 7));

    return 0;
}
