#ifndef CYANIDE_MEMORY_PROTECTION_HPP_
#define CYANIDE_MEMORY_PROTECTION_HPP_

#include <cstddef>

namespace cyanide {

enum class protection_type { read_write, read_write_execute };

class memory_protection {
public:
    memory_protection(
        void                    *address,
        std::size_t              size,
        cyanide::protection_type protection);
    ~memory_protection();

    memory_protection(const memory_protection &)            = delete;
    memory_protection &operator=(const memory_protection &) = delete;

    memory_protection(memory_protection &&other) noexcept;
    memory_protection &operator=(memory_protection &&other) noexcept;

    friend void swap(memory_protection &lhs, memory_protection &rhs) noexcept;

protected:
    void         *address_ = nullptr;
    std::size_t   size_{};
    unsigned long original_protection_{};
};

} // namespace cyanide

#endif // !CYANIDE_MEMORY_PROTECTION_HPP_
