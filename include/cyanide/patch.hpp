#ifndef CYANIDE_PATCH_HPP_
#define CYANIDE_PATCH_HPP_

#include <cyanide/defs.hpp>
#include <cyanide/memory_protection.hpp>

#include <algorithm> // std::copy_n
#include <array>
#include <concepts> // std::convertible_to
#include <cstddef>
#include <cstring> // std::memcpy
#include <optional>
#include <span>
#include <stdexcept>
#include <utility> // std::exchange, std::forward, std::move, std::swap
#include <vector>

namespace cyanide {

namespace detail {
    class patch_vector_storage;
} // namespace detail

/*
 * If you want to provide your own implementation of patch storage, it must have
 * the following traits:
 *
 * void store(std::span<const cyanide::byte_t>);
 * void restore(void *address);
 *
 * The first one to save the original bytes somewhere (it's up to you to decide
 * how to do it), and the second one to copy previously stored bytes to
 * destination address specified in the parameter.
 */

template <typename Storage = cyanide::detail::patch_vector_storage>
class patch : protected Storage {
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

    /*
     * See https://stackoverflow.com/a/35890646 for why <> is needed here.
     *
     * TL;DR - Without <> we're friending some specific non-template function,
     * i.e. if Storage is patch_vector_storage, the linker will search for
     * pretty concrete function - patch<patch_vector_storage>::swap and won't
     * consider the template function.
     *
     * For this reason we use <> to friend a template function.
     */
    friend void swap<>(patch<Storage> &lhs, patch<Storage> &rhs);

protected:
    void       *address_    = nullptr;
    std::size_t patch_size_ = 0;
    bool        unprotect_  = false;
};

namespace detail {
    class patch_vector_storage {
    protected:
        void store(std::span<const cyanide::byte_t> original_bytes)
        {
            original_bytes_.assign(
                original_bytes.begin(),
                original_bytes.end());
        }

        void restore(void *address)
        {
            std::memcpy(
                address,
                original_bytes_.data(),
                original_bytes_.size());
        }

        friend void swap(patch_vector_storage &lhs, patch_vector_storage &rhs)
        {
            using std::swap;

            swap(lhs.original_bytes_, rhs.original_bytes_);
        }

    private:
        std::vector<cyanide::byte_t> original_bytes_;
    };

    template <std::size_t N>
    class patch_array_storage {
    protected:
        void store(std::span<const cyanide::byte_t> original_bytes)
        {
            if (original_bytes.size() != original_bytes_.size())
            {
                throw std::out_of_range{
                    "Usage of array storage of inappropriate size"};
            }

            std::copy(
                original_bytes.begin(),
                original_bytes.end(),
                original_bytes_.begin());
        }

        void restore(void *address)
        {
            std::memcpy(
                address,
                original_bytes_.data(),
                original_bytes_.size());
        }

        friend void swap(patch_array_storage &lhs, patch_array_storage &rhs)
        {
            using std::swap;

            swap(lhs.original_bytes_, rhs.original_bytes_);
        }

    private:
        std::array<cyanide::byte_t, N> original_bytes_{};
    };

    template <typename T>
    concept byte_concept = requires(T t)
    // clang-format off
    {
        { t } -> std::convertible_to<cyanide::byte_t>;
    };
    // clang-format on

    template <typename Storage, cyanide::detail::byte_concept... T>
    auto make_patch_impl(void *address, T &&...patch_bytes)
    {
        static_assert(sizeof...(T) > 0, "Patch bytes have not been specified");

        const std::array<cyanide::byte_t, sizeof...(T)> patch_arr{
            static_cast<cyanide::byte_t>(std::forward<T>(patch_bytes))...};

        return patch<Storage>{address, patch_arr};
    }
} // namespace detail

/*
 * byte_concept is required to ensure that all the passed arguments are indeed
 * bytes. You can't pass any other parameters via the parameter pack as its used
 * to calculate the array size.
 */

/*
 * Construct a patch that stores the original bytes on heap using @p std::vector
 *
 * @param address Patch address.
 * @param patch_bytes Bytes to replace with (contents of the patch).
 */
template <cyanide::detail::byte_concept... T>
auto make_dynamic_patch(void *address, T &&...patch_bytes)
{
    return cyanide::detail::make_patch_impl<
        cyanide::detail::patch_vector_storage,
        T...>(address, std::forward<T>(patch_bytes)...);
}

/*
 * Construct a patch that stores the original bytes on stack using @p std::array
 *
 * @param address Patch address.
 * @param patch_bytes Bytes to replace with (contents of the patch).
 */
template <cyanide::detail::byte_concept... T>
auto make_static_patch(void *address, T &&...patch_bytes)
{
    return cyanide::detail::make_patch_impl<
        cyanide::detail::patch_array_storage<sizeof...(T)>,
        T...>(address, std::forward<T>(patch_bytes)...);
}

/************************************************
 * Implementation
 ***********************************************/

template <typename Storage>
patch<Storage>::patch(
    void                            *address,
    std::span<const cyanide::byte_t> patch_bytes,
    bool                             unprotect)
    : address_{address},
      patch_size_{patch_bytes.size()},
      unprotect_{unprotect}
{
    // Optionally unprotect the specified memory region till the end of the
    // scope
    std::optional<cyanide::memory_protection> protection;

    if (unprotect_)
        protection.emplace(
            address_,
            patch_size_,
            cyanide::protection_type::read_write);

    // Save the original bytes
    Storage::store(
        std::span{static_cast<const cyanide::byte_t *>(address_), patch_size_});

    // Apply the patch
    std::memcpy(address, patch_bytes.data(), patch_size_);
}

template <typename Storage>
patch<Storage>::~patch()
{
    std::optional<cyanide::memory_protection> protection;

    if (unprotect_)
        protection.emplace(address_, patch_size_, protection_type::read_write);

    // Undo the patch
    Storage::restore(address_);
}

template <typename Storage>
patch<Storage>::patch(patch &&other) noexcept
    : Storage{std::move(other)}, // <- This doesn't cause slicing, we're just
                                 // moving the base part of the class -
                                 // https://stackoverflow.com/a/22977905/8289462
      address_{std::exchange(other.address_, nullptr)},
      patch_size_{std::exchange(other.patch_size_, 0)},
      unprotect_{std::exchange(other.unprotect_, false)}
{}

template <typename Storage>
patch<Storage> &patch<Storage>::operator=(patch &&other) noexcept
{
    patch<Storage> tmp{std::move(other)};

    using std::swap;
    swap(tmp, *this);

    return *this;
}

template <typename Storage>
void swap(patch<Storage> &lhs, patch<Storage> &rhs)
{
    using std::swap;

    swap(static_cast<Storage &>(lhs), static_cast<Storage &>(rhs));

    swap(lhs.address_, rhs.address_);
    swap(lhs.patch_size_, rhs.patch_size_);
    swap(lhs.unprotect_, rhs.unprotect_);
}

} // namespace cyanide

#endif // !CYANIDE_PATCH_HPP_
