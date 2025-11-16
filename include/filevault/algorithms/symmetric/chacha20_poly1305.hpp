#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_CHACHA20_POLY1305_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_CHACHA20_POLY1305_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/aead.h>
#include <memory>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief ChaCha20-Poly1305 AEAD encryption
 * 
 * ChaCha20-Poly1305 is a modern AEAD cipher combining:
 * - ChaCha20 stream cipher (designed by Daniel J. Bernstein)
 * - Poly1305 MAC for authentication
 * 
 * Benefits over AES-GCM:
 * - No need for AES-NI hardware acceleration
 * - Constant-time implementation easier to achieve
 * - Better performance on software-only platforms
 * - Recommended by IETF (RFC 8439)
 * 
 * Security:
 * - 256-bit key
 * - 96-bit nonce (12 bytes) - MUST be unique per encryption
 * - 128-bit authentication tag (16 bytes)
 * - Provides confidentiality and authenticity
 */
class ChaCha20Poly1305 : public core::ICryptoAlgorithm {
public:
    ChaCha20Poly1305();
    virtual ~ChaCha20Poly1305() = default;
    
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
    
    size_t key_size() const override { return 32; }  // 256 bits
    size_t nonce_size() const { return 12; }         // 96 bits (RFC 8439)
    size_t tag_size() const { return 16; }           // 128 bits
    
    bool is_suitable_for(core::SecurityLevel level) const override;
};

} // namespace symmetric
} // namespace algorithms
} // namespace filevault

#endif // FILEVAULT_ALGORITHMS_SYMMETRIC_CHACHA20_POLY1305_HPP
