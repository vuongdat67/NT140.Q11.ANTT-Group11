/**
 * @file aes_ctr.hpp
 * @brief AES-CTR (Counter mode) encryption implementation
 *
 * AES in Counter mode - stream cipher mode.
 * NOT authenticated - use with HMAC for integrity protection.
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CTR_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CTR_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-CTR encryption (stream cipher mode, not authenticated)
 * 
 * Supports 128, 192, and 256 bit keys.
 * No padding needed (stream cipher).
 * Requires unique nonce/counter for each encryption.
 * 
 * @warning This mode does NOT provide authentication!
 */
class AES_CTR : public core::ICryptoAlgorithm {
public:
    explicit AES_CTR(size_t key_bits = 256);
    virtual ~AES_CTR() = default;
    
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
    size_t nonce_size() const { return 16; }  // Full block as IV/counter
    
    bool is_suitable_for(core::SecurityLevel level) const override;

private:
    size_t key_bits_;
    core::AlgorithmType type_;
    std::string botan_name_;
};

} // namespace symmetric
} // namespace algorithms
} // namespace filevault

#endif // FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CTR_HPP
