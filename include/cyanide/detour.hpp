#ifndef CYANIDE_DETOUR_HPP_
#define CYANIDE_DETOUR_HPP_

#include "func_traits.hpp"

#include <subhook.h>
#include <xbyak/xbyak.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility> // std::move, std::forward

namespace cyanide::Hooks {

using cyanide::Utils::FunctionPtr;

namespace Detail {
    template <typename, typename>
    struct thunk_wrapper {
    };

    template <typename HookPtrT, typename Ret, typename... Args>
    struct thunk_wrapper<HookPtrT, Ret(__cdecl *)(Args...)> {
        using SourceT = Ret(__cdecl *)(Args...);

        static Ret __cdecl func(
            HookPtrT       hook,
            std::uintptr_t return_addr,
            Args... args)
        {
            const auto callable_source =
                reinterpret_cast<SourceT>(hook->actual_hook_.GetTrampoline());

            return hook->callback_wrapper(
                callable_source,
                std::function{hook->callback_},
                std::forward<Args>(args)...);
        }
    };

    template <typename HookPtrT, typename Ret, typename... Args>
    struct thunk_wrapper<HookPtrT, Ret(__stdcall *)(Args...)> {
        using SourceT = Ret(__stdcall *)(Args...);

        static Ret __stdcall func(HookPtrT hook, Args... args)
        {
            const auto callable_source =
                reinterpret_cast<SourceT>(hook->actual_hook_.GetTrampoline());

            return hook->callback_wrapper(
                callable_source,
                std::function{hook->callback_},
                std::forward<Args>(args)...);
        }
    };

    template <typename HookPtrT, typename Ret, typename... Args>
    struct thunk_wrapper<HookPtrT, Ret(__thiscall *)(Args...)> {
        using SourceT = Ret(__thiscall *)(Args...);

        static Ret __stdcall func(HookPtrT hook, Args... args)
        {
            const auto callable_source =
                reinterpret_cast<SourceT>(hook->actual_hook_.GetTrampoline());

            return hook->callback_wrapper(
                callable_source,
                std::function{hook->callback_},
                std::forward<Args>(args)...);
        }
    };

    template <typename HookPtrT, typename Ret, typename... Args>
    struct thunk_wrapper<HookPtrT, Ret(__fastcall *)(Args...)> {
        using SourceT = Ret(__fastcall *)(Args...);

        // fastcall will pop the argument from the stack if it's a struct
        struct StackArg {
            HookPtrT arg;
        };

        static Ret __fastcall func(StackArg stack_arg, Args... args)
        {
            HookPtrT hook = stack_arg.arg;

            const auto callable_source =
                reinterpret_cast<SourceT>(hook->actual_hook_.GetTrampoline());

            return hook->callback_wrapper(
                callable_source,
                std::function{hook->callback_},
                std::forward<Args>(args)...);
        }
    };
} // namespace Detail

template <FunctionPtr SourceT, typename CallbackT>
class Detour {
    using HookPtrT = Detour<SourceT, CallbackT> *;

    friend struct Detail::thunk_wrapper<HookPtrT, SourceT>;

public:
    Detour(SourceT source, CallbackT callback)
        : source_{reinterpret_cast<std::uint8_t *>(source)},
          callback_{std::move(callback)}
    {
        constexpr std::size_t address_size_32_bit = 4;

        static_assert(
            sizeof(std::size_t) == address_size_32_bit,
            "Only 32-bit builds are supported");
    }

    void install()
    {
        if (!thunk_)
            thunk_ = make_thunk();

        if (!actual_hook_.Install(source_, const_cast<std::uint8_t *>(thunk_)))
        {
            throw std::runtime_error{"Failed to install the hook"};
        }
    }

    void uninstall()
    {
        if (!actual_hook_.Remove())
        {
            throw std::runtime_error{"Failed to uninstall the hook"};
        }
    }

protected:
    std::uint8_t *source_ = nullptr;
    CallbackT     callback_{};

    Xbyak::CodeGenerator code_gen_;
    const std::uint8_t * thunk_ = nullptr;
    subhook::Hook        actual_hook_;

    const std::uint8_t *make_thunk()
    {
        using namespace Xbyak::util;
        using namespace cyanide::Utils;

        constexpr auto source_conv = function_convention_v<SourceT>;

        code_gen_.reset();

        /*
         * Explaining the speciality of cdecl case
         *
         * If we take a general approach (as for other calling conventions) for
         * cdecl convention, the stack layout will look like this:
         *
         * [ orig return address ]  <-- top of the stack
         * [     hook object     ]
         * [         arg1        ]
         * [         arg2        ]
         * [         argN        ]
         *
         * But in cdecl the caller must do the clean up and if we return to the
         * original address, the clean up won't be done correctly (original code
         * has no clue about hook object, right?), that's why we can't just
         * return to the original. Instead, we don't do anything with original
         * return address and pushing our own one. That's explains why we have
         * unused `return_addr` parameter in cdecl thunk - original address is
         * treated as an argument.
         *
         * In other conventions the stack is cleaned up by the callee (i.e. the
         * called function), which is aware of additional "hook object" argument
         * and will do the clean up correctly and return to the original address
         * with no troubles.
         */

        if constexpr (source_conv != CallingConv::ccdecl)
            code_gen_.pop(eax);

        // Pass this pointer as argument
        if constexpr (source_conv == CallingConv::cthiscall)
            code_gen_.push(ecx);

        code_gen_.push(reinterpret_cast<std::uintptr_t>(this));

        if constexpr (source_conv == CallingConv::ccdecl)
        {
            code_gen_.call(&Detail::thunk_wrapper<HookPtrT, SourceT>::func);
            code_gen_.add(esp, 4);
            code_gen_.ret();
        }
        else
        {
            code_gen_.push(eax);
            code_gen_.jmp(&Detail::thunk_wrapper<HookPtrT, SourceT>::func);
        }

        return code_gen_.getCode();
    }

    template <typename Ret, typename... Args>
    static Ret callback_wrapper(
        SourceT                     source,
        std::function<Ret(Args...)> callback,
        Args &&...args)
    {
        return callback(std::forward<Args>(args)...);
    }

    template <typename Ret, typename... Args>
    static Ret callback_wrapper(
        SourceT                              source,
        std::function<Ret(SourceT, Args...)> callback,
        Args &&...args)
    {
        return callback(source, std::forward<Args>(args)...);
    }
};

} // namespace cyanide::Hooks

#endif // !CYANIDE_DETOUR_HPP_
