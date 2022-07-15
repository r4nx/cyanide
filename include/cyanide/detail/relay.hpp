#ifndef CYANIDE_RELAY_HPP_
#define CYANIDE_RELAY_HPP_

#include <cstdint>
#include <utility> // std::forward

namespace cyanide::detail {

template <typename, typename>
struct relay {};

template <typename HookWrapperT, typename Ret, typename... Args>
struct relay<HookWrapperT, Ret(__cdecl *)(Args...)> {
    using SourceT = Ret(__cdecl *)(Args...);

    static Ret __cdecl func(
        HookWrapperT  *hook_wrapper,
        std::uintptr_t return_addr,
        Args... args)
    {
        // See explanation why it's unused in make_relay() function
        static_cast<void>(return_addr);

        const auto callable_source = reinterpret_cast<SourceT>(
            hook_wrapper->hook_impl_->get_trampoline());

        return hook_wrapper->callback_dispatcher(
            callable_source,
            hook_wrapper->callback_,
            std::forward<Args>(args)...);
    }
};

template <typename HookWrapperT, typename Ret, typename... Args>
struct relay<HookWrapperT, Ret(__stdcall *)(Args...)> {
    using SourceT = Ret(__stdcall *)(Args...);

    static Ret __stdcall func(HookWrapperT *hook_wrapper, Args... args)
    {
        const auto callable_source = reinterpret_cast<SourceT>(
            hook_wrapper->hook_impl_->get_trampoline());

        return hook_wrapper->callback_dispatcher(
            callable_source,
            hook_wrapper->callback_,
            std::forward<Args>(args)...);
    }
};

template <typename HookWrapperT, typename Ret, typename... Args>
struct relay<HookWrapperT, Ret(__thiscall *)(Args...)> {
    using SourceT = Ret(__thiscall *)(Args...);

    static Ret __stdcall func(HookWrapperT *hook_wrapper, Args... args)
    {
        const auto callable_source = reinterpret_cast<SourceT>(
            hook_wrapper->hook_impl_->get_trampoline());

        return hook_wrapper->callback_dispatcher(
            callable_source,
            hook_wrapper->callback_,
            std::forward<Args>(args)...);
    }
};

template <typename HookWrapperT, typename Ret, typename... Args>
struct relay<HookWrapperT, Ret(__fastcall *)(Args...)> {
    using SourceT = Ret(__fastcall *)(Args...);

    // fastcall will pop the argument from the stack if it's a struct
    struct StackArg {
        HookWrapperT *arg;
    };

    static Ret __fastcall func(StackArg stack_arg, Args... args)
    {
        HookWrapperT *hook_wrapper = stack_arg.arg;

        const auto callable_source = reinterpret_cast<SourceT>(
            hook_wrapper->hook_impl_->get_trampoline());

        return hook_wrapper->callback_dispatcher(
            callable_source,
            hook_wrapper->callback_,
            std::forward<Args>(args)...);
    }
};

} // namespace cyanide::detail

#endif // !CYANIDE_RELAY_HPP_
