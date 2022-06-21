#ifndef CYANIDE_TYPES_FUNCTION_TRAITS_HPP_
#define CYANIDE_TYPES_FUNCTION_TRAITS_HPP_

#include <tuple>
#include <type_traits>

namespace cyanide::types {

template <typename T>
concept FunctionPtr = std::is_function_v<std::remove_pointer_t<T>>;

template <typename T>
concept Functor = requires(T a)
{
    a.operator();
};

// --------------------------------------------------------

enum class CallingConv { cthiscall, ccdecl, cstdcall, cfastcall };

template <typename>
struct function_convention {};

template <typename Ret, typename... Args>
struct function_convention<Ret(__cdecl *)(Args...)> {
    static constexpr CallingConv value = CallingConv::ccdecl;
};

template <typename Ret, typename... Args>
struct function_convention<Ret(__stdcall *)(Args...)> {
    static constexpr CallingConv value = CallingConv::cstdcall;
};

template <typename Ret, typename... Args>
struct function_convention<Ret(__thiscall *)(Args...)> {
    static constexpr CallingConv value = CallingConv::cthiscall;
};

template <typename Ret, typename Class, typename... Args>
struct function_convention<Ret (Class::*)(Args...)> {
    static constexpr CallingConv value = CallingConv::cthiscall;
};

template <typename Ret, typename... Args>
struct function_convention<Ret(__fastcall *)(Args...)> {
    static constexpr CallingConv value = CallingConv::cfastcall;
};

template <typename Func>
constexpr CallingConv function_convention_v = function_convention<Func>::value;

// ----------------------------------------------------------------------------

template <typename>
struct method_to_func {};

template <typename Ret, typename Class, typename... Args>
struct method_to_func<Ret (Class::*)(Args...)> {
    using type = Ret(Args...);
};

template <typename T>
using method_to_func_t = typename method_to_func<T>::type;

// ----------------------------------------------------------------------------

template <typename>
struct function_decompose {};

template <typename Ret, typename... Args>
struct function_decompose<Ret (*)(Args...)> {
    using return_type = Ret;
    using arguments   = std::tuple<Args...>;
};

template <typename Ret, typename Class, typename... Args>
struct function_decompose<Ret (Class::*)(Args...)>
    : function_decompose<Ret (*)(Args...)> {};

template <typename Ret, typename Class, typename... Args>
struct function_decompose<Ret (Class::*)(Args...) const>
    : function_decompose<Ret (Class::*)(Args...)> {};

template <Functor F>
struct function_decompose<F> : function_decompose<decltype(&F::operator())> {};

} // namespace cyanide::types

#endif // !CYANIDE_TYPES_FUNCTION_TRAITS_HPP_
