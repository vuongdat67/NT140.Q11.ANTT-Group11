/**
 * @file aes_cfb.hpp
 * @brief AES-CFB (Cipher Feedback) mode encryption
 * 
 * CFB mode is a stream cipher mode that turns a block cipher into a 
 * self-synchronizing stream cipher. It's similar to CBC but encrypts
 * the previous ciphertext block instead of XORing.
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CFB_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_CFB_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-CFB encryption algorithm
 * 
 * Features:
 * - Self-synchronizing stream cipher
 * - No padding required
 * - Can recover from bit errors (self-healing)
 * - Encryption and decryption use same operation
 */
class AES_CFB : public core::ICryptoAlgorithm {
public:
    /**
     * @brief Construct AES-CFB with specified key size
     * @param key_bits Key size in bits (128, 192, or 256)
     */
    explicit AES_CFB(size_t key_bits = 256);
    
    ~AES_CFB() override = default;
    
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
    
    bool requires_padding() const { return false; }
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
