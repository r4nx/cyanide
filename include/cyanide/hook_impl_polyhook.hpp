#ifndef CYANIDE_HOOK_IMPL_POLYHOOK_HPP_
#define CYANIDE_HOOK_IMPL_POLYHOOK_HPP_

#include <cyanide/function_traits.hpp>
#include <cyanide/hook_wrapper.hpp>

#include <polyhook2/Detour/ADetour.hpp>
#include <polyhook2/Detour/x86Detour.hpp>

#include <concepts>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <utility> // std::forward

namespace cyanide {

template <std::derived_from<PLH::Detour> HookT>
class polyhook_implementation {
public:
    void install(void *source, const void *destination)
    {
        detour_.emplace(
            reinterpret_cast<std::uint64_t>(source),
            reinterpret_cast<std::uint64_t>(destination),
            &trampoline_);

        if (!detour_->hook())
            throw std::runtime_error{
                std::format("Failed to install the hook at {}", source)};
    }

    void uninstall()
    {
        const bool result = detour_->unHook();
        detour_.reset();

        if (!result)
            throw std::runtime_error{"Failed to uninstall the hook"};
    }

    void *get_trampoline()
    {
        return reinterpret_cast<void *>(trampoline_);
    }

protected:
    std::optional<HookT> detour_;
    std::uint64_t        trampoline_ = 0;
};

template <typename... Args>
class polyhook_x86
    : public hook_wrapper<polyhook_implementation<PLH::x86Detour>, Args...> {
public:
    /*
     * Note that Args &&... here is not a forwarding reference but an rvalue,
     * because of class template instead of constructor template. To enable
     * forwarding reference the deduction guide below the class definition is
     * used.
     *
     * https://stackoverflow.com/a/64262244
     * https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
     */
    polyhook_x86(Args &&...args)
        : hook_wrapper<polyhook_implementation<PLH::x86Detour>, Args...>{
            std::forward<Args>(args)...}
    {}
};

template <typename... Args>
polyhook_x86(Args &&...) -> polyhook_x86<Args...>;

} // namespace cyanide

#endif // !CYANIDE_HOOK_IMPL_POLYHOOK_HPP_
