#ifndef FILEVAULT_CORE_TYPES_HPP
#define FILEVAULT_CORE_TYPES_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace filevault {
namespace core {

/**
 * @brief Enumeration of encryption algorithm types
 */
enum class AlgorithmType {
    // Symmetric ciphers
    AES_128_GCM,
    AES_192_GCM,
    AES_256_GCM,
    CHACHA20_POLY1305,
    
    // Hash functions
    SHA256,
    SHA512,
    BLAKE2B,
    
    // Classic (educational)
    CAESAR,
    VIGENERE,
    PLAYFAIR
};

/**
 * @brief Enumeration of Key Derivation Function types
 */
enum class KDFType {
    ARGON2ID,
    ARGON2I,
    PBKDF2_SHA256,
    PBKDF2_SHA512,
    SCRYPT
};

/**
 * @brief Security level determining algorithm parameters
 */
enum class SecurityLevel {
    WEAK,       // Fast, for testing (KDF: 10k iterations, 16MB memory)
    MEDIUM,     // Balanced (KDF: 100k iterations, 64MB memory)
    STRONG,     // High security (KDF: 200k iterations, 128MB memory)
    PARANOID    // Maximum (KDF: 500k iterations, 256MB memory)
};

/**
 * @brief Configuration for encryption operations
 */
struct EncryptionConfig {
    AlgorithmType algorithm = AlgorithmType::AES_256_GCM;
    KDFType kdf = KDFType::ARGON2ID;
    SecurityLevel level = SecurityLevel::MEDIUM;
    
    // KDF parameters (auto-set based on SecurityLevel)
    uint32_t kdf_iterations = 100000;
    uint32_t kdf_memory_kb = 65536;
    uint32_t kdf_parallelism = 4;
    
    // Encryption parameters (generated automatically or provided)
    std::vector<uint8_t> salt;
    std::optional<std::vector<uint8_t>> nonce;
    std::optional<std::vector<uint8_t>> tag;
    std::optional<std::vector<uint8_t>> associated_data;
    
    // Compression
    bool compress = false;
    int compression_level = 6;
    
    // Metadata
    bool include_metadata = true;
    std::string comment;
    
    /**
     * @brief Apply security level parameters
     */
    void apply_security_level();
};

} // namespace core
} // namespace filevault

#endif // FILEVAULT_CORE_TYPES_HPP
