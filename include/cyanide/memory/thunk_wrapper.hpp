#ifndef CYANIDE_MEMORY_THUNK_WRAPPER_HPP_
#define CYANIDE_MEMORY_THUNK_WRAPPER_HPP_

#include <cstdint>
#include <utility> // std::forward

namespace cyanide::memory::detail {

template <typename, typename>
struct ThunkWrapper {};

template <typename HookPtrT, typename Ret, typename... Args>
struct ThunkWrapper<HookPtrT, Ret(__cdecl *)(Args...)> {
    using SourceT = Ret(__cdecl *)(Args...);

    static Ret __cdecl func(
        HookPtrT       hook,
        std::uintptr_t return_addr,
        Args... args)
    {
        // See explanation why it's unused below in make_thunk() function
        static_cast<void>(return_addr);

        const auto callable_source =
            reinterpret_cast<SourceT>(hook->backend_->get_trampoline());

        return hook->callback_wrapper(
            callable_source,
            hook->callback_,
            std::forward<Args>(args)...);
    }
};

template <typename HookPtrT, typename Ret, typename... Args>
struct ThunkWrapper<HookPtrT, Ret(__stdcall *)(Args...)> {
    using SourceT = Ret(__stdcall *)(Args...);

    static Ret __stdcall func(HookPtrT hook, Args... args)
    {
        const auto callable_source =
            reinterpret_cast<SourceT>(hook->backend_->get_trampoline());

        return hook->callback_wrapper(
            callable_source,
            hook->callback_,
            std::forward<Args>(args)...);
    }
};

template <typename HookPtrT, typename Ret, typename... Args>
struct ThunkWrapper<HookPtrT, Ret(__thiscall *)(Args...)> {
    using SourceT = Ret(__thiscall *)(Args...);

    static Ret __stdcall func(HookPtrT hook, Args... args)
    {
        const auto callable_source =
            reinterpret_cast<SourceT>(hook->backend_->get_trampoline());

        return hook->callback_wrapper(
            callable_source,
            hook->callback_,
            std::forward<Args>(args)...);
    }
};

template <typename HookPtrT, typename Ret, typename... Args>
struct ThunkWrapper<HookPtrT, Ret(__fastcall *)(Args...)> {
    using SourceT = Ret(__fastcall *)(Args...);

    // fastcall will pop the argument from the stack if it's a struct
    struct StackArg {
        HookPtrT arg;
    };

    static Ret __fastcall func(StackArg stack_arg, Args... args)
    {
        HookPtrT hook = stack_arg.arg;

        const auto callable_source =
            reinterpret_cast<SourceT>(hook->backend_->get_trampoline());

        return hook->callback_wrapper(
            callable_source,
            hook->callback_,
            std::forward<Args>(args)...);
    }
};

} // namespace cyanide::memory::detail

#endif // !CYANIDE_MEMORY_THUNK_WRAPPER_HPP_
