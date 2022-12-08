#if !defined _WIN32
    #error "Unsupported platform"
#endif

#include <cyanide/memory_protection.hpp>

#include <Windows.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility> // std::exchange, std::move, std::swap

namespace cyanide {

memory_protection::memory_protection(
    void                    *address,
    std::size_t              size,
    cyanide::protection_type protection)
    : address_{address},
      size_{size}
{
    DWORD original_protection{};
    DWORD new_protection{};

    switch (protection)
    {
        case cyanide::protection_type::read_write:
            new_protection = PAGE_READWRITE;
            break;

        case cyanide::protection_type::read_write_execute:
            new_protection = PAGE_EXECUTE_READWRITE;
            break;
    }

    if (VirtualProtect(address, size, new_protection, &original_protection)
        == 0)
    {
        throw std::runtime_error{
            "Failed to apply the patch - VirtualProtect failed with error "
            "code "
            + std::to_string(GetLastError())};
    }

    original_protection_ = original_protection;
}

memory_protection::~memory_protection()
{
    // The object seems to be moved-from
    if (!address_)
        return;

    // Aggregate initialization here ensures the variables type compatibility by
    // prohibiting the narrowing conversion
    DWORD original_protection{original_protection_};

    VirtualProtect(address_, size_, original_protection, &original_protection);
}

memory_protection::memory_protection(memory_protection &&other) noexcept
    : address_{std::exchange(other.address_, nullptr)},
      size_{std::exchange(other.size_, 0)},
      original_protection_{std::exchange(other.original_protection_, 0)}
{}

memory_protection &
memory_protection::operator=(memory_protection &&other) noexcept
{
    memory_protection tmp{std::move(other)};

    using std::swap;
    swap(tmp, *this);

    return *this;
}

void swap(memory_protection &lhs, memory_protection &rhs) noexcept
{
    using std::swap;

    swap(lhs.address_, rhs.address_);
    swap(lhs.size_, rhs.size_);
    swap(lhs.original_protection_, rhs.original_protection_);
}

} // namespace cyanide
