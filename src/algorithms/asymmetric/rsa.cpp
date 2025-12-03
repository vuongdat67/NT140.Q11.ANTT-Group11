/**
 * @file rsa.cpp
 * @brief RSA asymmetric encryption implementation
 */

#include "filevault/algorithms/asymmetric/rsa.hpp"
#include <botan/auto_rng.h>
#include <botan/rsa.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>
#include <botan/x509_key.h>
#include <botan/data_src.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace asymmetric {

RSA::RSA(size_t key_bits) : key_bits_(key_bits) {
    if (key_bits != 2048 && key_bits != 3072 && key_bits != 4096) {
        throw std::invalid_argument("RSA key size must be 2048, 3072, or 4096 bits");
    }
    
    switch (key_bits) {
        case 2048:
            type_ = core::AlgorithmType::RSA_2048;
            break;
        case 3072:
            type_ = core::AlgorithmType::RSA_3072;
            break;
        case 4096:
        default:
            type_ = core::AlgorithmType::RSA_4096;
            break;
    }
    
    spdlog::debug("Created RSA-{} algorithm", key_bits);
}

std::string RSA::name() const {
    return "RSA-" + std::to_string(key_bits_);
}

core::AlgorithmType RSA::type() const {
    return type_;
}

RSAKeyPair RSA::generate_key_pair() {
    RSAKeyPair key_pair;
    key_pair.bits = key_bits_;
    
    try {
        Botan::AutoSeeded_RNG rng;
        Botan::RSA_PrivateKey private_key(rng, key_bits_);
        
        // Export private key as PKCS#8 PEM
        auto private_pem = Botan::PKCS8::PEM_encode(private_key);
        key_pair.private_key.assign(private_pem.begin(), private_pem.end());
        
        // Export public key as X.509 PEM
        auto public_pem = Botan::X509::PEM_encode(private_key);
        key_pair.public_key.assign(public_pem.begin(), public_pem.end());
        
        spdlog::debug("Generated RSA-{} key pair", key_bits_);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to generate RSA key pair: {}", e.what());
        throw;
    }
    
    return key_pair;
}

core::CryptoResult RSA::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    [[maybe_unused]] const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Check plaintext size limit
        if (plaintext.size() > max_plaintext_size()) {
            result.success = false;
            result.error_message = "Plaintext too large for RSA. Max: " + 
                std::to_string(max_plaintext_size()) + " bytes, got " + 
                std::to_string(plaintext.size());
            return result;
        }
        
        // Load public key
        std::string key_pem(key.begin(), key.end());
        Botan::DataSource_Memory key_source(key_pem);
        auto public_key = Botan::X509::load_key(key_source);
        
        if (!public_key) {
            result.success = false;
            result.error_message = "Failed to load RSA public key";
            return result;
        }
        
        // Create encryptor with OAEP padding (SHA-256)
        Botan::AutoSeeded_RNG rng;
        Botan::PK_Encryptor_EME encryptor(*public_key, rng, "EME-OAEP(SHA-256)");
        
        // Encrypt
        auto ciphertext = encryptor.encrypt(plaintext.data(), plaintext.size(), rng);
        
        result.data.assign(ciphertext.begin(), ciphertext.end());
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("RSA encryption: {} bytes -> {} bytes", plaintext.size(), result.data.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("RSA encryption error: ") + e.what();
        spdlog::error("RSA encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult RSA::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    [[maybe_unused]] const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Load private key
        std::string key_pem(key.begin(), key.end());
        Botan::DataSource_Memory key_source(key_pem);
        auto private_key = Botan::PKCS8::load_key(key_source);
        
        if (!private_key) {
            result.success = false;
            result.error_message = "Failed to load RSA private key";
            return result;
        }
        
        // Create decryptor with OAEP padding (SHA-256)
        Botan::AutoSeeded_RNG rng;
        Botan::PK_Decryptor_EME decryptor(*private_key, rng, "EME-OAEP(SHA-256)");
        
        // Decrypt
        auto plaintext = decryptor.decrypt(ciphertext.data(), ciphertext.size());
        
        result.data.assign(plaintext.begin(), plaintext.end());
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("RSA decryption: {} bytes -> {} bytes", ciphertext.size(), result.data.size());
        
    } catch (const Botan::Decoding_Error& e) {
        result.success = false;
        result.error_message = "RSA decryption failed: invalid key or corrupted data";
        spdlog::warn("RSA decryption: decoding error");
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("RSA decryption error: ") + e.what();
        spdlog::error("RSA decryption failed: {}", e.what());
    }
    
    return result;
}

std::vector<uint8_t> RSA::sign(
    std::span<const uint8_t> data,
    std::span<const uint8_t> private_key
) {
    try {
        std::string key_pem(private_key.begin(), private_key.end());
        Botan::DataSource_Memory key_source(key_pem);
        auto priv_key = Botan::PKCS8::load_key(key_source);
        
        if (!priv_key) {
            throw std::runtime_error("Failed to load private key");
        }
        
        Botan::AutoSeeded_RNG rng;
        Botan::PK_Signer signer(*priv_key, rng, "EMSA-PSS(SHA-256)");
        
        signer.update(data.data(), data.size());
        auto signature = signer.signature(rng);
        
        return std::vector<uint8_t>(signature.begin(), signature.end());
        
    } catch (const std::exception& e) {
        spdlog::error("RSA signing failed: {}", e.what());
        throw;
    }
}

bool RSA::verify(
    std::span<const uint8_t> data,
    std::span<const uint8_t> signature,
    std::span<const uint8_t> public_key
) {
    try {
        std::string key_pem(public_key.begin(), public_key.end());
        Botan::DataSource_Memory key_source(key_pem);
        auto pub_key = Botan::X509::load_key(key_source);
        
        if (!pub_key) {
            return false;
        }
        
        Botan::PK_Verifier verifier(*pub_key, "EMSA-PSS(SHA-256)");
        verifier.update(data.data(), data.size());
        
        return verifier.check_signature(signature.data(), signature.size());
        
    } catch (const std::exception& e) {
        spdlog::warn("RSA verification failed: {}", e.what());
        return false;
    }
}

bool RSA::is_suitable_for(core::SecurityLevel level) const {
    switch (level) {
        case core::SecurityLevel::WEAK:
            return key_bits_ >= 2048;
        case core::SecurityLevel::MEDIUM:
            return key_bits_ >= 2048;
        case core::SecurityLevel::STRONG:
            return key_bits_ >= 3072;
        case core::SecurityLevel::PARANOID:
            return key_bits_ >= 4096;
        default:
            return false;
    }
}

} // namespace asymmetric
} // namespace algorithms
} // namespace filevault
