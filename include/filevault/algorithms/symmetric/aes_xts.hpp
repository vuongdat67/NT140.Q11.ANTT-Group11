/**
 * @file aes_xts.hpp
 * @brief AES-XTS (XEX-based Tweaked-codebook mode with ciphertext Stealing)
 * 
 * XTS mode is specifically designed for disk encryption.
 * It uses two keys and a "tweak" value (typically sector number).
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_XTS_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_XTS_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-XTS encryption algorithm
 * 
 * Features:
 * - Designed for disk/sector encryption
 * - Uses two keys (effectively double key size)
 * - Tweak value provides sector-level randomization
 * - No ciphertext expansion (same size as plaintext)
 * - Not suitable for streaming data
 */
class AES_XTS : public core::ICryptoAlgorithm {
public:
    /**
     * @brief Construct AES-XTS with specified key size
     * @param key_bits Key size per cipher in bits (128 or 256)
     *        Total key is 2x this (256 or 512 bits)
     */
    explicit AES_XTS(size_t key_bits = 256);
    
    ~AES_XTS() override = default;
    
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
    
    // XTS uses double key (two AES keys)
    size_t key_size() const override { return (key_bits_ / 8) * 2; }
    size_t tweak_size() const { return 16; }  // Tweak/IV size
    size_t block_size() const { return 16; }
    
    bool requires_padding() const { return false; }  // Uses ciphertext stealing
    bool is_authenticated() const { return false; }
    bool is_suitable_for(core::SecurityLevel level) const override;
    
private:
    size_t key_bits_;
    core::AlgorithmType type_;
    std::string botan_name_;
};

} // namespace symmetric
} // namespace algorithms
} // namespace filevault

#endif
