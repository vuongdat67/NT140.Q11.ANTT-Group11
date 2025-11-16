#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_AES_GCM_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_AES_GCM_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/aead.h>
#include <memory>

namespace filevault {
namespace algorithms {
namespace symmetric {

/**
 * @brief AES-GCM AEAD encryption
 */
class AES_GCM : public core::ICryptoAlgorithm {
public:
    explicit AES_GCM(size_t key_bits = 256);
    virtual ~AES_GCM() = default;
    
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
    size_t nonce_size() const { return 12; } // GCM standard
    size_t tag_size() const { return 16; }    // 128-bit tag
    
    bool is_suitable_for(core::SecurityLevel level) const override;

private:
    size_t key_bits_;
    core::AlgorithmType type_;
    std::string botan_name_;
};

} // namespace symmetric
} // namespace algorithms
} // namespace filevault

#endif // FILEVAULT_ALGORITHMS_SYMMETRIC_AES_GCM_HPP
