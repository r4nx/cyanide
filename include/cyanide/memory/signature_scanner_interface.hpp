#ifndef CYANIDE_MEMORY_SIGNATURE_SCANNER_INTERFACE_HPP_
#define CYANIDE_MEMORY_SIGNATURE_SCANNER_INTERFACE_HPP_

#include <cyanide/misc/defs.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace cyanide::memory {

struct Signature {
    std::vector<cyanide::byte_t> pattern;
    std::string_view             mask;
    std::size_t                  offset = 0;
};

class SignatureScannerInterface {
public:
    virtual ~SignatureScannerInterface() = default;

    virtual cyanide::byte_t *scan(const Signature &signature) = 0;
};

} // namespace cyanide::memory

#endif // !CYANIDE_MEMORY_SIGNATURE_SCANNER_INTERFACE_HPP_
