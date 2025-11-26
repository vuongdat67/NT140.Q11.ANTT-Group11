/**
 * @file aes_ecb.hpp
 * @brief AES-ECB (Electronic Codebook) mode encryption
 * 
 * WARNING: ECB mode is NOT secure for most purposes!
 * Each block is encrypted independently, which leaks patterns.
 * This is provided for educational purposes and legacy compatibility only.
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_ECB_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_ECB_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-ECB encryption algorithm
 * 
 * ⚠️ SECURITY WARNING: ECB mode is insecure!
 * - Identical plaintext blocks produce identical ciphertext blocks
 * - Patterns in plaintext are visible in ciphertext
 * - Use CBC, CTR, or GCM instead for real security
 * 
 * Only use for:
 * - Educational purposes
 * - Single block encryption
 * - Legacy system compatibility
 */
class AES_ECB : public core::ICryptoAlgorithm {
public:
    /**
     * @brief Construct AES-ECB with specified key size
     * @param key_bits Key size in bits (128, 192, or 256)
     */
    explicit AES_ECB(size_t key_bits = 256);
    
    ~AES_ECB() override = default;
    
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
    size_t block_size() const { return 16; }
    
    bool requires_padding() const { return true; }
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
