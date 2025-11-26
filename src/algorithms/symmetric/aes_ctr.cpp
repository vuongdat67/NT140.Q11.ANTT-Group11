/**
 * @file aes_ctr.cpp
 * @brief AES-CTR encryption implementation
 */

#include "filevault/algorithms/symmetric/aes_ctr.hpp"
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

AES_CTR::AES_CTR(size_t key_bits) : key_bits_(key_bits) {
    if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        throw std::invalid_argument("AES-CTR key size must be 128, 192, or 256 bits");
    }
    
    switch (key_bits) {
        case 128:
            type_ = core::AlgorithmType::AES_128_CTR;
            botan_name_ = "AES-128/CTR";
            break;
        case 192:
            type_ = core::AlgorithmType::AES_192_CTR;
            botan_name_ = "AES-192/CTR";
            break;
        case 256:
        default:
            type_ = core::AlgorithmType::AES_256_CTR;
            botan_name_ = "AES-256/CTR";
            break;
    }
    
    spdlog::debug("Created AES-{}-CTR algorithm", key_bits);
}

std::string AES_CTR::name() const {
    return "AES-" + std::to_string(key_bits_) + "-CTR";
}

core::AlgorithmType AES_CTR::type() const {
    return type_;
}

core::CryptoResult AES_CTR::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate key size
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size. Expected " + 
                std::to_string(key_size()) + " bytes, got " + 
                std::to_string(key.size());
            return result;
        }
        
        // Generate or use provided nonce
        std::vector<uint8_t> nonce;
        if (config.nonce.has_value() && config.nonce.value().size() == nonce_size()) {
            nonce = config.nonce.value();
            spdlog::debug("AES-CTR: Using provided nonce");
        } else {
            Botan::AutoSeeded_RNG rng;
            nonce.resize(nonce_size());
            rng.randomize(nonce.data(), nonce.size());
            spdlog::debug("AES-CTR: Generated new nonce ({} bytes)", nonce.size());
        }
        
        // Create cipher
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-CTR cipher";
            return result;
        }
        
        // Set key and nonce
        cipher->set_key(key.data(), key.size());
        cipher->start(nonce);
        
        // Encrypt (no padding in CTR mode)
        Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
        cipher->finish(buffer);
        
        // Store result
        result.data.assign(buffer.begin(), buffer.end());
        result.nonce = std::move(nonce);
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("AES-{}-CTR encryption: {} bytes -> {} bytes in {:.2f}ms",
                      key_bits_, plaintext.size(), result.data.size(), 
                      result.processing_time_ms);
        
    } catch (const Botan::Exception& e) {
        result.success = false;
        result.error_message = std::string("Botan error: ") + e.what();
        spdlog::error("AES-CTR encryption failed: {}", e.what());
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-CTR encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult AES_CTR::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate key size
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size. Expected " + 
                std::to_string(key_size()) + " bytes, got " + 
                std::to_string(key.size());
            return result;
        }
        
        // Get nonce from config
        if (!config.nonce.has_value() || config.nonce.value().size() != nonce_size()) {
            result.success = false;
            result.error_message = "Nonce must be provided for CTR decryption (16 bytes)";
            return result;
        }
        
        auto& nonce = config.nonce.value();
        
        // Create cipher (CTR encryption and decryption are the same)
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-CTR cipher";
            return result;
        }
        
        // Set key and nonce
        cipher->set_key(key.data(), key.size());
        cipher->start(nonce);
        
        // Decrypt
        Botan::secure_vector<uint8_t> buffer(ciphertext.begin(), ciphertext.end());
        cipher->finish(buffer);
        
        // Store result
        result.data.assign(buffer.begin(), buffer.end());
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("AES-{}-CTR decryption: {} bytes -> {} bytes in {:.2f}ms",
                      key_bits_, ciphertext.size(), result.data.size(), 
                      result.processing_time_ms);
        
    } catch (const Botan::Exception& e) {
        result.success = false;
        result.error_message = std::string("Botan error: ") + e.what();
        spdlog::error("AES-CTR decryption failed: {}", e.what());
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-CTR decryption failed: {}", e.what());
    }
    
    return result;
}

bool AES_CTR::is_suitable_for(core::SecurityLevel level) const {
    // CTR mode without authentication is not recommended for high security
    switch (level) {
        case core::SecurityLevel::WEAK:
        case core::SecurityLevel::MEDIUM:
            return true;
        case core::SecurityLevel::STRONG:
        case core::SecurityLevel::PARANOID:
            return false;  // Prefer AEAD modes
        default:
            return false;
    }
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
