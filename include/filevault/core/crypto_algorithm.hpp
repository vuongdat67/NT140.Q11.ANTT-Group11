#ifndef FILEVAULT_CORE_CRYPTO_ALGORITHM_HPP
#define FILEVAULT_CORE_CRYPTO_ALGORITHM_HPP

#include <string>
#include <span>
#include "types.hpp"
#include "result.hpp"

namespace filevault {
namespace core {

/**
 * @brief Interface for cryptographic algorithms
 */
class ICryptoAlgorithm {
public:
    virtual ~ICryptoAlgorithm() = default;
    
    /**
     * @brief Get algorithm name
     */
    virtual std::string name() const = 0;
    
    /**
     * @brief Get algorithm type
     */
    virtual AlgorithmType type() const = 0;
    
    /**
     * @brief Encrypt data
     * @param plaintext Input data to encrypt
     * @param key Encryption key (derived from password)
     * @param config Encryption configuration
     * @return Encrypted data with metadata
     */
    virtual CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const EncryptionConfig& config
    ) = 0;
    
    /**
     * @brief Decrypt data
     * @param ciphertext Encrypted data
     * @param key Decryption key (derived from password)
     * @param config Decryption configuration
     * @return Decrypted data
     */
    virtual CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const EncryptionConfig& config
    ) = 0;
    
    /**
     * @brief Get recommended key size in bytes
     */
    virtual size_t key_size() const = 0;
    
    /**
     * @brief Check if algorithm is suitable for security level
     */
    virtual bool is_suitable_for(SecurityLevel level) const = 0;
};

} // namespace core
} // namespace filevault

#endif // FILEVAULT_CORE_CRYPTO_ALGORITHM_HPP
