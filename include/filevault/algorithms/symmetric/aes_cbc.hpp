/**
 * @file aes_cbc.hpp
 * @brief AES-CBC encryption implementation
 *
 * AES in Cipher Block Chaining mode - classic block cipher mode.
 * NOT authenticated - use with HMAC for integrity protection.
 *
 * @warning CBC mode is NOT AEAD - no built-in authentication!
 *          Consider using AES-GCM instead for authenticated encryption.
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CBC_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CBC_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-CBC encryption (legacy mode, not authenticated)
 * 
 * Supports 128, 192, and 256 bit keys.
 * Uses PKCS7 padding.
 * Requires unique IV for each encryption.
 * 
 * @warning This mode does NOT provide authentication!
 *          Ciphertext can be modified without detection.
 *          Consider using AES-GCM for authenticated encryption.
 */
class AES_CBC : public core::ICryptoAlgorithm {
public:
    explicit AES_CBC(size_t key_bits = 256);
    virtual ~AES_CBC() = default;
    
    std::string name() const override;
    core::AlgorithmType type() const override;
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    size_t key_size() const override { return key_bits_ / 8; }
    size_t iv_size() const { return 16; }  // AES block size
    size_t block_size() const { return 16; }
    
    bool is_suitable_for(core::SecurityLevel level) const override;

private:
    size_t key_bits_;
    core::AlgorithmType type_;
    std::string botan_name_;
};

} // namespace symmetric
} // namespace algorithms
} // namespace filevault

#endif // FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CBC_HPP
