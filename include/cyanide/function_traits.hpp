#ifndef CYANIDE_FUNCTION_TRAITS_HPP_
#define CYANIDE_FUNCTION_TRAITS_HPP_

#include <functional> // std::function
#include <tuple>
#include <type_traits>
#include <utility> // std::declval

namespace cyanide::types {

template <typename T>
concept FunctionPtr = std::is_function_v<std::remove_pointer_t<T>>;

template <typename T>
concept Functor = requires(T a)
{
    a.operator();
};

// ----------------------------------------------------------------------------

enum class calling_conv { cthiscall, ccdecl, cstdcall, cfastcall };

template <typename>
struct function_convention {};

template <typename Ret, typename... Args>
struct function_convention<Ret(__cdecl *)(Args...)> {
    static constexpr calling_conv value = calling_conv::ccdecl;
};

template <typename Ret, typename... Args>
struct function_convention<Ret(__stdcall *)(Args...)> {
    static constexpr calling_conv value = calling_conv::cstdcall;
};

template <typename Ret, typename... Args>
struct function_convention<Ret(__thiscall *)(Args...)> {
    static constexpr calling_conv value = calling_conv::cthiscall;
};

template <typename Ret, typename Class, typename... Args>
struct function_convention<Ret (Class::*)(Args...)> {
    static constexpr calling_conv value = calling_conv::cthiscall;
};

template <typename Ret, typename... Args>
struct function_convention<Ret(__fastcall *)(Args...)> {
    static constexpr calling_conv value = calling_conv::cfastcall;
};

template <typename Func>
inline constexpr calling_conv function_convention_v =
    function_convention<Func>::value;

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

template <typename T>
using result_type_t = typename decltype(std::function{
    std::declval<T>()})::result_type;

// ----------------------------------------------------------------------------

// https://github.com/allwanttokissme/Yet-another-hook-library/blob/7b01b8d8a8473ae49713d7a9663b70baae50dcce/hook_function_traits.hpp#L146
// https://stackoverflow.com/q/65722989/8289462
template <typename>
struct function_decompose {};

template <typename Ret, typename... Args>
struct function_decompose<Ret (*)(Args...)> {
    using signature   = Ret(Args...);
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

#endif // !CYANIDE_FUNCTION_TRAITS_HPP_
