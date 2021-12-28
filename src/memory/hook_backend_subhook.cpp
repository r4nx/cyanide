#include <cyanide/memory/hook_backend_subhook.hpp>

namespace cyanide::memory {

void SubhookDetourBackend::install(void *source, const void *destination)
{
    hook_.Install(source, const_cast<void *>(destination));
}

void SubhookDetourBackend::uninstall()
{
    hook_.Remove();
}

void *SubhookDetourBackend::get_trampoline()
{
    return hook_.GetTrampoline();
}

} // namespace cyanide::memory
