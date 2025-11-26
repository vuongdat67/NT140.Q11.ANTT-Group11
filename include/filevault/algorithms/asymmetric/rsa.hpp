/**
 * @file rsa.hpp
 * @brief RSA asymmetric encryption
 * 
 * RSA (Rivest-Shamir-Adleman) is an asymmetric cryptographic algorithm
 * used for secure data transmission and digital signatures.
 */

#ifndef FILEVAULT_ALGORITHMS_ASYMMETRIC_RSA_HPP
#define FILEVAULT_ALGORITHMS_ASYMMETRIC_RSA_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <botan/rsa.h>
#include <botan/pubkey.h>

namespace filevault {
namespace algorithms {
namespace asymmetric {

/**
 * @brief RSA key pair
 */
struct RSAKeyPair {
    std::vector<uint8_t> public_key;   // PEM or DER encoded
    std::vector<uint8_t> private_key;  // PEM or DER encoded
    size_t bits;
};

/**
 * @brief RSA asymmetric encryption algorithm
 * 
 * Features:
 * - Public key encryption, private key decryption
 * - Key sizes: 2048, 3072, 4096 bits
 * - OAEP padding (PKCS#1 v2.1) for encryption
 * - PSS padding for signatures
 * 
 * Note: RSA encrypts limited data (key_size - padding overhead).
 * For large data, use hybrid encryption (RSA + symmetric cipher).
 */
class RSA : public core::ICryptoAlgorithm {
public:
    /**
     * @brief Construct RSA with specified key size
     * @param key_bits Key size in bits (2048, 3072, or 4096)
     */
    explicit RSA(size_t key_bits = 2048);
    
    ~RSA() override = default;
    
    std::string name() const override;
    core::AlgorithmType type() const override;
    
    /**
     * @brief Encrypt with public key
     * @param plaintext Data to encrypt (limited by key size)
     * @param key Public key (PEM or DER encoded)
     * @param config Encryption configuration
     */
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    /**
     * @brief Decrypt with private key
     * @param ciphertext Data to decrypt
     * @param key Private key (PEM or DER encoded)
     * @param config Encryption configuration
     */
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    /**
     * @brief Generate a new RSA key pair
     * @return RSAKeyPair containing public and private keys
     */
    RSAKeyPair generate_key_pair();
    
    /**
     * @brief Sign data with private key
     * @param data Data to sign
     * @param private_key Private key (PEM encoded)
     * @return Signature bytes
     */
    std::vector<uint8_t> sign(
        std::span<const uint8_t> data,
        std::span<const uint8_t> private_key
    );
    
    /**
     * @brief Verify signature with public key
     * @param data Original data
     * @param signature Signature to verify
     * @param public_key Public key (PEM encoded)
     * @return true if signature is valid
     */
    bool verify(
        std::span<const uint8_t> data,
        std::span<const uint8_t> signature,
        std::span<const uint8_t> public_key
    );
    
    size_t key_size() const override { return key_bits_ / 8; }
    
    /**
     * @brief Maximum plaintext size that can be encrypted
     * For OAEP with SHA-256: key_bytes - 2*hash_len - 2 = key_bytes - 66
     */
    size_t max_plaintext_size() const { return (key_bits_ / 8) - 66; }
    
    bool requires_padding() const { return true; }  // OAEP padding
    bool is_authenticated() const { return false; }
    bool is_suitable_for(core::SecurityLevel level) const override;
    
private:
    size_t key_bits_;
    core::AlgorithmType type_;
};

} // namespace asymmetric
} // namespace algorithms
} // namespace filevault

#endif
