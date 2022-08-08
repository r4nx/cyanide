#ifndef CYANIDE_HOOK_WRAPPER_HPP_
#define CYANIDE_HOOK_WRAPPER_HPP_

#include <cyanide/defs.hpp>
#include <cyanide/detail/relay.hpp>
#include <cyanide/function_traits.hpp>

#include <xbyak/xbyak.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility> // std::exchange, std::forward, std::move, std::swap

namespace cyanide {

namespace types {
    template <typename T>
    concept HookConcept = std::default_initializable<T> && requires(
        T           hook_impl,
        void       *source,
        const void *destination)
    {
        hook_impl.install(source, destination);
        hook_impl.uninstall();

        // Enable clang format when concepts will be handled properly
        // clang-format off
        { hook_impl.get_trampoline() } -> std::convertible_to<void *>;
        // clang-format on
    };
} // namespace types

template <
    cyanide::types::HookConcept HookT,
    typename SourceT,
    typename CallbackT>
class hook_wrapper {
    using this_t = hook_wrapper<HookT, SourceT, CallbackT>;

    friend struct cyanide::detail::relay<this_t, SourceT>;

public:
    template <typename... HookArgs>
    hook_wrapper(SourceT source, CallbackT callback, HookArgs &&...hook_args)
        : source_{reinterpret_cast<cyanide::byte_t *>(source)},
          callback_{std::move(callback)}
    {
        static_assert(
            sizeof(std::size_t) == 4,
            "Only 32-bit builds are supported");

        hook_impl_ =
            std::make_unique<HookT>(std::forward<HookArgs>(hook_args)...);
        code_gen_ = std::make_unique<Xbyak::CodeGenerator>();
    }

    ~hook_wrapper()
    {
        hook_impl_->uninstall();
    }

    hook_wrapper(const hook_wrapper &)            = delete;
    hook_wrapper &operator=(const hook_wrapper &) = delete;

    hook_wrapper(hook_wrapper &&other)
        : source_{std::exchange(other.source_, nullptr)},
          relay_jump_{std::exchange(other.relay_jump_, nullptr)},
          callback_{std::move(other.callback_)},
          hook_impl_{std::move(other.hook_impl_)},
          code_gen_{std::move(other.code_gen_)}
    {}

    hook_wrapper &operator=(hook_wrapper &&other)
    {
        using std::swap;

        /*
         * Friend swap function defined below must be present for this in order
         * to work. Otherwise std::swap will be used and we'll end up with a
         * stack overflow, because std::swap is implemented with the use of move
         * assignment (which we are currently trying to implement)
         */
        hook_wrapper tmp{std::move(other)};
        swap(tmp, *this);

        return *this;
    }

    friend void swap(hook_wrapper &lhs, hook_wrapper &rhs)
    {
        using std::swap;

        swap(lhs.source_, rhs.source_);
        swap(lhs.relay_, lhs.relay_);
        swap(lhs.callback_, rhs.callback_);
        swap(lhs.hook_impl_, rhs.hook_impl_);
        swap(lhs.code_gen_, rhs.code_gen_);
    }

    void install()
    {
        if (!relay_jump_)
            relay_jump_ = make_relay_jump();

        hook_impl_->install(source_, relay_jump_);
    }

    void uninstall()
    {
        hook_impl_->uninstall();
    }

    void *get_trampoline()
    {
        return hook_impl_->get_trampoline();
    }

protected:
    cyanide::byte_t       *source_     = nullptr;
    const cyanide::byte_t *relay_jump_ = nullptr;

    /*
     * There are 2 ways to retrieve the CallbackT signature - by decomposing it
     * via type traits, and by simulating the initialization of std::function.
     * First one may not work for some cases (like overloaded function?), but
     * allows us to use std::move_only_function, which, in turn, lets the
     * move-only types be used as a callback.
     *
     * As for the second one - to store some callable, the template parameters
     * of std::function have to be specified in field definition, i.e. here. In
     * case if the callable is lambda (or some other functor), deduction guides
     * must be envolved. To use them, we simulate the std::function
     * initialization and retrieve the result type.
     *
     * Currently, only the second one is used due to issues with function
     * decomposing.
     *
     * https://stackoverflow.com/a/53673648
     */
    decltype(std::function{std::declval<CallbackT>()}) callback_;

    std::unique_ptr<HookT>                hook_impl_;
    std::unique_ptr<Xbyak::CodeGenerator> code_gen_;

    const cyanide::byte_t *make_relay_jump()
    {
        using namespace Xbyak::util;
        using namespace cyanide::types;

        constexpr auto source_conv = function_convention_v<SourceT>;

        // Is the return value passed as hidden out parameter instead of being
        // returned through registers
        constexpr bool hidden_param_return =
            get_type_size<result_type_t<SourceT>>() > 8;

        code_gen_->reset();

        /*
         * Explaining the speciality of cdecl case
         *
         * If we take a general approach (as for other calling conventions)
         * for cdecl convention, the stack layout will look like this:
         *
         * [ orig return address ]  <-- top of the stack
         * [     hook object     ]
         * [         arg1        ]
         * [         arg2        ]
         * [         argN        ]
         *
         * But in cdecl the caller must do the clean up and if we return to
         * the original address, the clean up won't be done correctly
         * (original code has no clue about hook object, right?), that's why
         * we can't just return to the original. Instead, we don't do
         * anything with original return address and pushing our own one.
         * That's explains why we have unused `return_addr` parameter in
         * cdecl thunk - original address is treated as an argument.
         *
         * In other conventions the stack is cleaned up by the callee (i.e.
         * the called function), which is aware of additional "hook object"
         * argument and will do the clean up correctly and return to the
         * original address with no troubles.
         */

        if constexpr (source_conv == calling_conv::ccdecl)
        {
            // Swap the hidden output parameter with return address
            if constexpr (hidden_param_return)
            {
                code_gen_->pop(eax);
                code_gen_->pop(edx);
                code_gen_->push(eax);
            }
        }
        else
        {
            code_gen_->pop(eax);

            if constexpr (hidden_param_return)
                code_gen_->pop(edx);
        }

        // Pass this pointer as argument
        if constexpr (source_conv == calling_conv::cthiscall)
            code_gen_->push(ecx);

        code_gen_->push(reinterpret_cast<std::uintptr_t>(this));

        if constexpr (hidden_param_return)
            code_gen_->push(edx);

        if constexpr (source_conv == calling_conv::ccdecl)
        {
            code_gen_->call(&detail::relay<this_t, SourceT>::func);

            if constexpr (hidden_param_return)
            {
                // Remove hook object from the stack, swap the hidden output
                // parameter with return address back
                code_gen_->pop(eax);
                code_gen_->add(esp, 4);
                code_gen_->pop(edx);
                code_gen_->push(eax);
                code_gen_->push(edx);
            }
            else
            {
                code_gen_->add(esp, 4);
            }

            code_gen_->ret();
        }
        else
        {
            code_gen_->push(eax);
            code_gen_->jmp(&detail::relay<this_t, SourceT>::func);
        }

        return code_gen_->getCode();
    }

    template <typename Ret, typename... Args>
    static Ret callback_dispatcher(
        SourceT                               source,
        std::function<Ret(SourceT, Args...)> &callback,
        Args &&...args)
    {
        return callback(source, std::forward<Args>(args)...);
    }

    template <typename Ret, typename... Args>
    static Ret callback_dispatcher(
        SourceT                      source,
        std::function<Ret(Args...)> &callback,
        Args &&...args)
    {
        return callback(std::forward<Args>(args)...);
    }
};

} // namespace cyanide

#endif // !CYANIDE_HOOK_WRAPPER_HPP_
