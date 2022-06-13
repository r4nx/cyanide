# cyanide

Library comprising the utilities for memory-related hacking like
hooks, signature scanning, patching, etc.

## Prerequisites
1. If you're going to use the subhook backend, install
[Yasm](https://yasm.tortall.net) and put its directory in the `PATH`.
1. Grab the cyanide in a way you like: submodule, installing, FetchContent
(preferred).

## Usage
### Hooks
Hooks are divided into two parts: frontend and backend. Any third-party hooking
library can be a **backend**, given you implement a `DetourBackendInterface`
for it. It simply redirects the program flow to another routine with the same
parameters and exactly same calling convention. This is not as convenient as
you would like it to be - in most cases you may need to transmit some context
to the hook callback, which could be done by making the callback a member
function - but that isn't possible as member function has a different calling
convention.

**Frontend** part to the rescue! It generates a wrapper with the necessary
calling convention, that, in turn, will call your callback. And in this case
the callback may be everything that can be placed in `std::function` -
a plain function, a lambda, result of an `std::bind`, etc.

Finally, an example:

```c++
int func_to_hook(int x, int y)
{
    if (x == 0)
        return 0;

    x *= 2;
    y *= 3;

    return y / x;
}

cyanide::memory::SubhookDetourBackend backend;
cyanide::memory::DetourFrontend       frontend{
    &backend,
    &func_to_hook,
    [](decltype(&func_to_hook) orig, int x, int y) -> int {
        return orig(x, y) + 5;
    }};

frontend.install();
```

Note that receiving an `orig` parameter in the callback is optional - if you
don't want to call the original function you may just omit it.
