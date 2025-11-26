/**
 * @file aes_ofb.hpp
 * @brief AES-OFB (Output Feedback) mode encryption
 * 
 * OFB mode is a stream cipher mode that generates a keystream
 * independent of plaintext/ciphertext. Pre-computation possible.
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_OFB_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_OFB_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-OFB encryption algorithm
 * 
 * Features:
 * - True stream cipher mode
 * - No error propagation (bit flip affects only one bit)
 * - Keystream can be pre-computed
 * - No padding required
 */
class AES_OFB : public core::ICryptoAlgorithm {
public:
    /**
     * @brief Construct AES-OFB with specified key size
     * @param key_bits Key size in bits (128, 192, or 256)
     */
    explicit AES_OFB(size_t key_bits = 256);
    
    ~AES_OFB() override = default;
    
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
    size_t iv_size() const { return 16; }
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
