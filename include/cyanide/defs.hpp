#ifndef CYANIDE_DEFS_HPP_
#define CYANIDE_DEFS_HPP_

#include <cstddef>
#include <type_traits>

namespace cyanide {

using byte_t = unsigned char;

template <typename T>
constexpr std::size_t get_type_size()
{
    if constexpr (std::is_same_v<T, void>)
        return 0;
    else
        return sizeof(T);
}

} // namespace cyanide

#endif // !CYANIDE_DEFS_HPP_
