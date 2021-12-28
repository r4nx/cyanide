#ifndef CYANIDE_MEMORY_HOOK_BACKEND_SUBHOOK_HPP_
#define CYANIDE_MEMORY_HOOK_BACKEND_SUBHOOK_HPP_

#include <cyanide/memory/hook_backend_interface.hpp>

#include <subhook.h>

namespace cyanide::memory {

class SubhookDetourBackend : public DetourBackendInterface {
public:
    void  install(void *source, const void *destination) override;
    void  uninstall() override;
    void *get_trampoline() override;

protected:
    subhook::Hook hook_;
};

} // namespace cyanide::memory

#endif // !CYANIDE_MEMORY_HOOK_BACKEND_SUBHOOK_HPP_
