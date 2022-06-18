#ifndef CYANIDE_MEMORY_HOOK_FRONTEND_HPP_
#define CYANIDE_MEMORY_HOOK_FRONTEND_HPP_

#include <cyanide/memory/hook_backend_interface.hpp>
#include <cyanide/memory/thunk_wrapper.hpp>
#include <cyanide/misc/defs.hpp>
#include <cyanide/types/function_traits.hpp>

#include <xbyak/xbyak.h>

#include <cstddef>
#include <cstdint>
#include <functional> // std::function
#include <memory>
#include <stdexcept>
#include <utility> // std::exchange, std::forward, std::move, std::swap

namespace cyanide::memory {

template <cyanide::types::FunctionPtr SourceT, typename CallbackT>
class DetourFrontend {
    using HookPtrT = DetourFrontend<SourceT, CallbackT> *;

    friend struct detail::ThunkWrapper<HookPtrT, SourceT>;

public:
    DetourFrontend(
        DetourBackendInterface *backend,
        SourceT                 source,
        CallbackT               callback)
        : backend_{backend},
          source_{reinterpret_cast<cyanide::byte_t *>(source)},
          callback_{std::move(callback)}
    {
        constexpr std::size_t address_size_32_bit = 4;

        static_assert(
            sizeof(std::size_t) == address_size_32_bit,
            "Only 32-bit builds are supported");

        code_gen_ = std::make_unique<Xbyak::CodeGenerator>();
    }

    ~DetourFrontend()
    {
        if (backend_)
            backend_->uninstall();
    }

    DetourFrontend(const DetourFrontend &)            = delete;
    DetourFrontend &operator=(const DetourFrontend &) = delete;

    DetourFrontend(DetourFrontend &&other)
        : backend_{std::exchange(other.backend_, nullptr)},
          source_{std::exchange(other.source_, nullptr)},
          callback_{std::move(other.callback_)},
          code_gen_{std::move(other.code_gen_)},
          thunk_{std::exchange(other.thunk_, nullptr)}
    {}

    DetourFrontend &operator=(DetourFrontend &&other)
    {
        using std::swap;

        /*
         * Friend swap function defined below must be present for this in order
         * to work. Otherwise std::swap will be used and we'll end up with a
         * stack overflow, because std::swap is implemented with the use of move
         * assignment (which we are currently trying to implement)
         */
        DetourFrontend tmp{std::move(other)};
        swap(tmp, *this);

        return *this;
    }

    friend void swap(DetourFrontend &lhs, DetourFrontend &rhs)
    {
        using std::swap;

        swap(lhs.backend_, rhs.backend_);
        swap(lhs.source_, rhs.source_);
        swap(lhs.callback_, rhs.callback_);
        swap(lhs.code_gen_, rhs.code_gen_);
        swap(lhs.thunk_, rhs.thunk_);
    }

    void install()
    {
        if (!thunk_)
            thunk_ = make_thunk();

        backend_->install(source_, thunk_);
    }

    void uninstall()
    {
        backend_->uninstall();
    }

protected:
    DetourBackendInterface *backend_ = nullptr;
    cyanide::byte_t        *source_  = nullptr;

    /*
     * To store some callable the template parameters of std::function have to
     * be specified in field defitions, i.e. here. In case if the callable is
     * lambda (or some other functor), deduction guides must be envolved. To use
     * them, we simulate the std::function initialization and retrieve the
     * result type.
     */
    decltype(std::function{std::declval<CallbackT>()}) callback_;

    std::unique_ptr<Xbyak::CodeGenerator> code_gen_;
    const cyanide::byte_t                *thunk_ = nullptr;

    const cyanide::byte_t *make_thunk()
    {
        using namespace Xbyak::util;
        using namespace cyanide::types;

        constexpr auto source_conv = function_convention_v<SourceT>;

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

        if constexpr (source_conv != CallingConv::ccdecl)
            code_gen_->pop(eax);

        // Pass this pointer as argument
        if constexpr (source_conv == CallingConv::cthiscall)
            code_gen_->push(ecx);

        code_gen_->push(reinterpret_cast<std::uintptr_t>(this));

        if constexpr (source_conv == CallingConv::ccdecl)
        {
            code_gen_->call(&detail::ThunkWrapper<HookPtrT, SourceT>::func);
            code_gen_->add(esp, 4);
            code_gen_->ret();
        }
        else
        {
            code_gen_->push(eax);
            code_gen_->jmp(&detail::ThunkWrapper<HookPtrT, SourceT>::func);
        }

        return code_gen_->getCode();
    }

    template <typename Ret, typename... Args>
    static Ret callback_wrapper(
        SourceT                                     source,
        const std::function<Ret(SourceT, Args...)> &callback,
        Args &&...args)
    {
        return callback(source, std::forward<Args>(args)...);
    }

    template <typename Ret, typename... Args>
    static Ret callback_wrapper(
        SourceT                            source,
        const std::function<Ret(Args...)> &callback,
        Args &&...args)
    {
        return callback(std::forward<Args>(args)...);
    }
};

} // namespace cyanide::memory

#endif // !CYANIDE_MEMORY_HOOK_FRONTEND_HPP_
