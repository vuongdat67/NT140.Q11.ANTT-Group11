#ifndef FILEVAULT_UTILS_CRYPTO_UTILS_HPP
#define FILEVAULT_UTILS_CRYPTO_UTILS_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <span>

namespace filevault {
namespace utils {

/**
 * @brief Crypto utility functions
 */
class CryptoUtils {
public:
    /**
     * @brief Encode bytes to hexadecimal string
     */
    static std::string hex_encode(std::span<const uint8_t> data);
    
    /**
     * @brief Decode hexadecimal string to bytes
     */
    static std::vector<uint8_t> hex_decode(const std::string& hex);
    
    /**
     * @brief Format bytes as human-readable size
     */
    static std::string format_bytes(size_t bytes);
};

} // namespace utils
} // namespace filevault

#endif // FILEVAULT_UTILS_CRYPTO_UTILS_HPP
