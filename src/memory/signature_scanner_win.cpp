#include <cyanide/memory/signature_scanner_win.hpp>
#include <cyanide/misc/defs.hpp>
#include <cyanide/types/safe_pun.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

#include <Windows.h>

namespace cyanide::memory {

WinAPISignatureScanner::WinAPISignatureScanner(const void *mod)
{
    MEMORY_BASIC_INFORMATION info{};

    if (!VirtualQuery(mod, &info, sizeof info))
        throw std::runtime_error{
            "VirtualQuery failed with return code "
            + std::to_string(GetLastError())};

    base = static_cast<cyanide::byte_t *>(info.AllocationBase);

    const auto dos = cyanide::types::safe_pun<IMAGE_DOS_HEADER>(base);
    const auto pe =
        cyanide::types::safe_pun<IMAGE_NT_HEADERS>(base + dos.e_lfanew);

    if (pe.Signature != IMAGE_NT_SIGNATURE)
        throw std::runtime_error{"NT signature check failed"};

    size = pe.OptionalHeader.SizeOfImage;
}

cyanide::byte_t *WinAPISignatureScanner::scan(const Signature &signature)
{
    if (signature.pattern.size() != signature.mask.size())
        throw std::logic_error{
            "Pattern size (" + std::to_string(signature.pattern.size())
            + ") doesn't match the mask size ("
            + std::to_string(signature.mask.size()) + ")."};

    auto       current_byte = base;
    const auto last_byte    = base + size;
    const auto pattern_size = signature.pattern.size();

    for (; current_byte < last_byte; ++current_byte)
    {
        std::size_t i{};

        for (i = 0; i < pattern_size; ++i)
        {
            // Scanning is out of range
            if (current_byte + i >= last_byte)
                break;

            // Scanning failed
            if (current_byte[i] != signature.pattern[i]
                && signature.mask[i] != '?')
                break;
        }

        // Scanning succeed
        if (i == pattern_size)
            return current_byte;
    }

    return nullptr;
}

} // namespace cyanide::memory
