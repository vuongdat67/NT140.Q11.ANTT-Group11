#include "filevault/utils/crypto_utils.hpp"
#include <botan/hex.h>
#include <fmt/core.h>

namespace filevault {
namespace utils {

std::string CryptoUtils::hex_encode(std::span<const uint8_t> data) {
    return Botan::hex_encode(data.data(), data.size());
}

std::vector<uint8_t> CryptoUtils::hex_decode(const std::string& hex) {
    return Botan::hex_decode(hex);
}

std::string CryptoUtils::format_bytes(size_t bytes) {
    constexpr size_t KB = 1024;
    constexpr size_t MB = KB * 1024;
    constexpr size_t GB = MB * 1024;
    
    if (bytes >= GB) {
        return fmt::format("{:.2f} GB", static_cast<double>(bytes) / GB);
    } else if (bytes >= MB) {
        return fmt::format("{:.2f} MB", static_cast<double>(bytes) / MB);
    } else if (bytes >= KB) {
        return fmt::format("{:.2f} KB", static_cast<double>(bytes) / KB);
    } else {
        return fmt::format("{} bytes", bytes);
    }
}

} // namespace utils
} // namespace filevault
