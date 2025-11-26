#ifndef FILEVAULT_ALGORITHMS_POST_QUANTUM_HPP
#define FILEVAULT_ALGORITHMS_POST_QUANTUM_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <string>
#include <vector>

namespace filevault {
namespace algorithms {
namespace pqc {

/**
 * @brief Key pair for post-quantum algorithms
 */
struct PQKeyPair {
    std::vector<uint8_t> public_key;
    std::vector<uint8_t> private_key;
    std::string algorithm;
};

/**
 * @brief Kyber Key Encapsulation Mechanism (KEM)
 * 
 * NIST FIPS 203 (ML-KEM) - Post-quantum secure key encapsulation
 * 
 * Kyber is a lattice-based KEM that provides:
 * - IND-CCA2 security
 * - Resistance to quantum computer attacks
 * - Efficient key generation, encapsulation, and decapsulation
 * 
 * Variants:
 * - Kyber-512: ~128-bit post-quantum security
 * - Kyber-768: ~192-bit post-quantum security (recommended)
 * - Kyber-1024: ~256-bit post-quantum security
 */
class Kyber : public core::ICryptoAlgorithm {
public:
    enum class Variant {
        Kyber512,   // AES-128 equivalent
        Kyber768,   // AES-192 equivalent (recommended)
        Kyber1024   // AES-256 equivalent
    };
    
    explicit Kyber(Variant variant = Variant::Kyber768);
    ~Kyber() override = default;
    
    std::string name() const override;
    core::AlgorithmType type() const override;
    
    /**
     * @brief Encapsulate: Generate shared secret and ciphertext
     * 
     * Uses recipient's public key to generate a shared secret
     * and encapsulated ciphertext.
     * 
     * @param plaintext Not used (KEM doesn't encrypt directly)
     * @param key Recipient's public key
     * @param config Configuration
     * @return CryptoResult with encapsulated ciphertext and shared secret in 'data'
     */
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    /**
     * @brief Decapsulate: Recover shared secret from ciphertext
     * 
     * Uses own private key to recover the shared secret.
     * 
     * @param ciphertext Encapsulated ciphertext
     * @param key Own private key
     * @param config Configuration
     * @return CryptoResult with shared secret in 'data'
     */
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    size_t key_size() const override;
    bool is_suitable_for(core::SecurityLevel level) const override;
    
    /**
     * @brief Generate Kyber key pair
     */
    PQKeyPair generate_keypair();
    
    /**
     * @brief Get public key size for this variant
     */
    size_t public_key_size() const;
    
    /**
     * @brief Get private key size for this variant
     */
    size_t private_key_size() const;
    
    /**
     * @brief Get ciphertext size for this variant
     */
    size_t ciphertext_size() const;
    
    /**
     * @brief Get shared secret size (always 32 bytes)
     */
    size_t shared_secret_size() const { return 32; }

private:
    Variant variant_;
    std::string botan_name_;
    core::AlgorithmType type_;
};

/**
 * @brief Dilithium Digital Signature Algorithm
 * 
 * NIST FIPS 204 (ML-DSA) - Post-quantum secure digital signatures
 * 
 * Dilithium is a lattice-based signature scheme that provides:
 * - EUF-CMA security
 * - Resistance to quantum computer attacks
 * - Efficient signing and verification
 * 
 * Variants:
 * - Dilithium2: ~128-bit post-quantum security
 * - Dilithium3: ~192-bit post-quantum security (recommended)
 * - Dilithium5: ~256-bit post-quantum security
 */
class Dilithium {
public:
    enum class Variant {
        Dilithium2,  // Level 2 security
        Dilithium3,  // Level 3 security (recommended)
        Dilithium5   // Level 5 security
    };
    
    explicit Dilithium(Variant variant = Variant::Dilithium3);
    
    std::string name() const;
    
    /**
     * @brief Generate Dilithium key pair
     */
    PQKeyPair generate_keypair();
    
    /**
     * @brief Sign a message
     * @param message Message to sign
     * @param private_key Signer's private key
     * @return Signature
     */
    std::vector<uint8_t> sign(
        std::span<const uint8_t> message,
        std::span<const uint8_t> private_key
    );
    
    /**
     * @brief Verify a signature
     * @param message Original message
     * @param signature Signature to verify
     * @param public_key Signer's public key
     * @return true if signature is valid
     */
    bool verify(
        std::span<const uint8_t> message,
        std::span<const uint8_t> signature,
        std::span<const uint8_t> public_key
    );
    
    /**
     * @brief Get public key size for this variant
     */
    size_t public_key_size() const;
    
    /**
     * @brief Get private key size for this variant
     */
    size_t private_key_size() const;
    
    /**
     * @brief Get signature size for this variant
     */
    size_t signature_size() const;

private:
    Variant variant_;
    std::string botan_name_;
};

/**
 * @brief Hybrid encryption combining Kyber with AES-GCM
 * 
 * This provides defense-in-depth by combining:
 * - Kyber for post-quantum secure key encapsulation
 * - AES-256-GCM for symmetric encryption
 * 
 * Even if one algorithm is broken, the other provides protection.
 */
class KyberHybrid : public core::ICryptoAlgorithm {
public:
    explicit KyberHybrid(Kyber::Variant variant = Kyber::Variant::Kyber768);
    ~KyberHybrid() override = default;
    
    std::string name() const override;
    core::AlgorithmType type() const override;
    
    /**
     * @brief Encrypt data using Kyber+AES-GCM hybrid
     * 
     * 1. Encapsulate with Kyber to get shared secret
     * 2. Use shared secret as AES key
     * 3. Encrypt data with AES-256-GCM
     * 
     * @param plaintext Data to encrypt
     * @param key Recipient's Kyber public key
     * @param config Configuration
     * @return Ciphertext containing Kyber ciphertext + AES ciphertext
     */
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    /**
     * @brief Decrypt data using Kyber+AES-GCM hybrid
     * 
     * 1. Decapsulate Kyber ciphertext to get shared secret
     * 2. Use shared secret as AES key
     * 3. Decrypt data with AES-256-GCM
     * 
     * @param ciphertext Encrypted data
     * @param key Own Kyber private key
     * @param config Configuration
     * @return Decrypted plaintext
     */
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    size_t key_size() const override;
    bool is_suitable_for(core::SecurityLevel level) const override;
    
    /**
     * @brief Generate hybrid key pair
     */
    PQKeyPair generate_keypair();

private:
    Kyber kyber_;
};

} // namespace pqc
} // namespace algorithms
} // namespace filevault

#endif // FILEVAULT_ALGORITHMS_POST_QUANTUM_HPP
