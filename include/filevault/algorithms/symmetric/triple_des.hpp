/**
 * @file triple_des.hpp
 * @brief Triple-DES (3DES) encryption implementation
 *
 * Triple-DES provides 112-bit effective security (168-bit key).
 * Legacy algorithm - use AES for new applications.
 */

#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_TRIPLE_DES_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_TRIPLE_DES_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/cipher_mode.h>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief Triple-DES (3DES/TDEA) encryption
 * 
 * Uses three DES operations: Encrypt-Decrypt-Encrypt (EDE)
 * 168-bit key (112 bits effective security)
 * 64-bit block size
 * 
 * @warning Legacy algorithm! Prefer AES for new applications.
 *          Block size of 64 bits is vulnerable to birthday attacks
 *          on large datasets (Sweet32 attack).
 */
class TripleDES : public core::ICryptoAlgorithm {
public:
    explicit TripleDES();
    virtual ~TripleDES() = default;
    
    std::string name() const override { return "3DES-CBC"; }
    core::AlgorithmType type() const override { return core::AlgorithmType::TRIPLE_DES_CBC; }
    
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
    
    size_t key_size() const override { return 24; }  // 168 bits (3 x 56 bits)
    size_t iv_size() const { return 8; }  // 64-bit block
    size_t block_size() const { return 8; }
    
    bool is_suitable_for(core::SecurityLevel level) const override;
};

} // namespace symmetric
} // namespace algorithms
} // namespace filevault

#endif // FILEVAULT_ALGORITHMS_SYMMETRIC_TRIPLE_DES_HPP
