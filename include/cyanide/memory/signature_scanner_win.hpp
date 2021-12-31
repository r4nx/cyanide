#ifndef CYANIDE_MEMORY_SIGNATURE_SCANNER_WIN_HPP_
#define CYANIDE_MEMORY_SIGNATURE_SCANNER_WIN_HPP_

#include <cyanide/memory/signature_scanner_interface.hpp>
#include <cyanide/misc/defs.hpp>

namespace cyanide::memory {

class WinAPISignatureScanner : public SignatureScannerInterface {
public:
    WinAPISignatureScanner(const void *mod);

    cyanide::byte_t *scan(const Signature &signature) override;

private:
    cyanide::byte_t *base = nullptr;
    std::size_t      size = 0;
};

} // namespace cyanide::memory

#endif // !CYANIDE_MEMORY_SIGNATURE_SCANNER_WIN_HPP_
