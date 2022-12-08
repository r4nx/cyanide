#ifndef CYANIDE_PATCH_HPP_
#define CYANIDE_PATCH_HPP_

#include <cyanide/defs.hpp>
#include <cyanide/memory_protection.hpp>

#include <algorithm> // std::copy_n
#include <array>
#include <concepts> // std::convertible_to
#include <cstddef>
#include <cstring>    // std::memcpy
#include <functional> // std::invoke
#include <iterator>   // std::back_inserter
#include <optional>
#include <span>
#include <utility> // std::exchange, std::forward, std::move, std::swap
#include <vector>

namespace cyanide {

template <template <typename...> typename Container = std::vector>
class patch {
public:
    patch(
        void                            *address,
        std::span<const cyanide::byte_t> patch_bytes,
        bool                             unprotect = true);

    ~patch();

    patch(const patch &)            = delete;
    patch &operator=(const patch &) = delete;

    patch(patch &&other) noexcept;
    patch &operator=(patch &&other) noexcept;

    friend void swap(patch &lhs, patch &rhs);

protected:
    void                      *address_ = nullptr;
    Container<cyanide::byte_t> original_bytes_{};
    bool                       unprotect_ = false;
};

namespace detail {
    // https://stackoverflow.com/a/11377375/8289462
    template <std::size_t N>
    struct array_size_binder {
        template <typename T>
        struct type : public std::array<T, N> {};
    };

    template <typename T>
    concept byte_concept = requires(T t)
    // clang-format off
    {
        { t } -> std::convertible_to<cyanide::byte_t>;
    };
    // clang-format on

    template <typename T>
    concept already_allocated = requires(T t)
    {
        requires t.size() > 0;
    };
} // namespace detail

/*
 * Construct a patch that stores the original bytes on stack using @p std::array
 *
 * @param address Patch address.
 * @param patch_bytes Bytes to replace with (contents of the patch).
 *
 * byte_concept is required to ensure that all the passed arguments are indeed
 * bytes. You can't pass any other parameters via the parameter pack as its used
 * to calculate the array size.
 */
template <cyanide::detail::byte_concept... T>
auto make_static_patch(void *address, T &&...patch_bytes)
{
    static_assert(sizeof...(T) > 0, "Patch bytes have not been specified");

    const std::array<cyanide::byte_t, sizeof...(T)> patch_arr{
        static_cast<cyanide::byte_t>(std::forward<T>(patch_bytes))...};

    return patch<
        typename cyanide::detail::array_size_binder<sizeof...(T)>::type>{
        address,
        patch_arr};
}

/************************************************
 * Implementation
 ***********************************************/

template <template <typename...> typename Container>
patch<Container>::patch(
    void                            *address,
    std::span<const cyanide::byte_t> patch_bytes,
    bool                             unprotect)
    : address_{address},
      unprotect_{unprotect}
{
    // Optionally unprotect the specified memory region till the end of the
    // scope
    std::optional<cyanide::memory_protection> protection;

    if (unprotect_)
        protection.emplace(
            address_,
            patch_bytes.size(),
            cyanide::protection_type::read_write);

    // Save the original bytes

    // https://stackoverflow.com/a/72423554
    // https://www.cppstories.com/2016/11/iife-for-complex-initialization/
    const auto orig_bytes_it = std::invoke([&] {
        if constexpr (cyanide::detail::already_allocated<
                          decltype(original_bytes_)>)
            return original_bytes_.begin();
        else
            return std::back_inserter(original_bytes_);
    });

    std::copy_n(
        static_cast<cyanide::byte_t *>(address),
        patch_bytes.size(),
        orig_bytes_it);

    // Apply the patch
    std::memcpy(address, patch_bytes.data(), patch_bytes.size());
}

template <template <typename...> typename Container>
patch<Container>::~patch()
{
    std::optional<cyanide::memory_protection> protection;

    if (unprotect_)
        protection.emplace(
            address_,
            original_bytes_.size(),
            protection_type::read_write);

    // Undo the patch
    std::memcpy(address_, original_bytes_.data(), original_bytes_.size());
}

template <template <typename...> typename Container>
patch<Container>::patch(patch &&other) noexcept
    : address_{std::exchange(other.address_, nullptr)},
      original_bytes_{std::move(other.original_bytes_)},
      unprotect_{std::exchange(other.unprotect_, false)}
{}

template <template <typename...> typename Container>
patch<Container> &patch<Container>::operator=(patch &&other) noexcept
{
    patch<Container> tmp{std::move(other)};

    using std::swap;
    swap(tmp, *this);

    return *this;
}

template <template <typename...> typename Container>
void swap(patch<Container> &lhs, patch<Container> &rhs)
{
    using std::swap;

    swap(lhs.address_, rhs.address_);
    swap(lhs.original_bytes_, rhs.original_bytes_);
    swap(lhs.unprotect_, rhs.unprotect_);
}

} // namespace cyanide

#endif // !CYANIDE_PATCH_HPP_
