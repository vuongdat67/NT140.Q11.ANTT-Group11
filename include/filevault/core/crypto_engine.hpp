#ifndef FILEVAULT_CORE_CRYPTO_ENGINE_HPP
#define FILEVAULT_CORE_CRYPTO_ENGINE_HPP

#include <memory>
#include <map>
#include <optional>
#include "crypto_algorithm.hpp"
#include "types.hpp"
#include "result.hpp"

namespace filevault {
namespace core {

/**
 * @brief Main cryptographic engine
 * Manages algorithms and provides key derivation
 */
class CryptoEngine {
public:
    CryptoEngine();
    ~CryptoEngine();
    
    // Prevent copying
    CryptoEngine(const CryptoEngine&) = delete;
    CryptoEngine& operator=(const CryptoEngine&) = delete;
    
    /**
     * @brief Initialize engine with default algorithms
     */
    void initialize();
    
    /**
     * @brief Register a custom algorithm
     */
    void register_algorithm(std::unique_ptr<ICryptoAlgorithm> algorithm);
    
    /**
     * @brief Get algorithm by type
     */
    ICryptoAlgorithm* get_algorithm(AlgorithmType type);
    
    /**
     * @brief Derive key from password using KDF
     * @param password User password
     * @param salt Random salt (MUST be unique per file)
     * @param config Configuration containing KDF parameters
     * @return Derived key
     */
    std::vector<uint8_t> derive_key(
        const std::string& password,
        const std::vector<uint8_t>& salt,
        const EncryptionConfig& config
    );
    
    /**
     * @brief Generate random salt (32 bytes default)
     */
    static std::vector<uint8_t> generate_salt(size_t length = 32);
    
    /**
     * @brief Generate random nonce/IV
     * @param length Nonce length (12 for GCM, 16 for CBC)
     */
    static std::vector<uint8_t> generate_nonce(size_t length = 12);
    
    /**
     * @brief Get algorithm name from type
     */
    static std::string algorithm_name(AlgorithmType type);
    
    /**
     * @brief Get KDF name from type
     */
    static std::string kdf_name(KDFType type);
    
    /**
     * @brief Get security level name from type
     */
    static std::string security_level_name(SecurityLevel level);
    
    /**
     * @brief Parse algorithm from string
     */
    static std::optional<AlgorithmType> parse_algorithm(const std::string& name);
    
    /**
     * @brief Parse KDF from string
     */
    static std::optional<KDFType> parse_kdf(const std::string& name);
    
    /**
     * @brief Parse security level from string
     */
    static std::optional<SecurityLevel> parse_security_level(const std::string& name);

private:
    std::map<AlgorithmType, std::unique_ptr<ICryptoAlgorithm>> algorithms_;
};

} // namespace core
} // namespace filevault

#endif // FILEVAULT_CORE_CRYPTO_ENGINE_HPP
