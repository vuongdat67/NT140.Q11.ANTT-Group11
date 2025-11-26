/**
 * @file aes_cbc.cpp
 * @brief AES-CBC encryption implementation
 */

#include "filevault/algorithms/symmetric/aes_cbc.hpp"
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

AES_CBC::AES_CBC(size_t key_bits) : key_bits_(key_bits) {
    if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        throw std::invalid_argument("AES-CBC key size must be 128, 192, or 256 bits");
    }
    
    switch (key_bits) {
        case 128:
            type_ = core::AlgorithmType::AES_128_CBC;
            botan_name_ = "AES-128/CBC/PKCS7";
            break;
        case 192:
            type_ = core::AlgorithmType::AES_192_CBC;
            botan_name_ = "AES-192/CBC/PKCS7";
            break;
        case 256:
        default:
            type_ = core::AlgorithmType::AES_256_CBC;
            botan_name_ = "AES-256/CBC/PKCS7";
            break;
    }
    
    spdlog::debug("Created AES-{}-CBC algorithm", key_bits);
}

std::string AES_CBC::name() const {
    return "AES-" + std::to_string(key_bits_) + "-CBC";
}

core::AlgorithmType AES_CBC::type() const {
    return type_;
}

core::CryptoResult AES_CBC::encrypt(
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
        
        // Generate or use provided IV
        std::vector<uint8_t> iv;
        if (config.nonce.has_value() && config.nonce.value().size() == iv_size()) {
            iv = config.nonce.value();
            spdlog::debug("AES-CBC: Using provided IV");
        } else {
            Botan::AutoSeeded_RNG rng;
            iv.resize(iv_size());
            rng.randomize(iv.data(), iv.size());
            spdlog::debug("AES-CBC: Generated new IV ({} bytes)", iv.size());
        }
        
        // Create cipher
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-CBC cipher";
            return result;
        }
        
        // Set key and IV
        cipher->set_key(key.data(), key.size());
        cipher->start(iv);
        
        // Encrypt (will add PKCS7 padding)
        Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
        cipher->finish(buffer);
        
        // Store result
        result.data.assign(buffer.begin(), buffer.end());
        result.nonce = std::move(iv);  // IV stored in nonce field
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("AES-{}-CBC encryption: {} bytes -> {} bytes in {:.2f}ms",
                      key_bits_, plaintext.size(), result.data.size(), 
                      result.processing_time_ms);
        
    } catch (const Botan::Exception& e) {
        result.success = false;
        result.error_message = std::string("Botan error: ") + e.what();
        spdlog::error("AES-CBC encryption failed: {}", e.what());
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-CBC encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult AES_CBC::decrypt(
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
        
        // Get IV from config
        if (!config.nonce.has_value() || config.nonce.value().size() != iv_size()) {
            result.success = false;
            result.error_message = "IV must be provided for CBC decryption (16 bytes)";
            return result;
        }
        
        auto& iv = config.nonce.value();
        
        // Validate ciphertext (must be multiple of block size)
        if (ciphertext.size() == 0 || ciphertext.size() % block_size() != 0) {
            result.success = false;
            result.error_message = "Invalid ciphertext size (must be multiple of 16)";
            return result;
        }
        
        // Create cipher
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Decryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-CBC cipher";
            return result;
        }
        
        // Set key and IV
        cipher->set_key(key.data(), key.size());
        cipher->start(iv);
        
        // Decrypt (will remove PKCS7 padding)
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
        
        spdlog::debug("AES-{}-CBC decryption: {} bytes -> {} bytes in {:.2f}ms",
                      key_bits_, ciphertext.size(), result.data.size(), 
                      result.processing_time_ms);
        
    } catch (const Botan::Decoding_Error&) {
        result.success = false;
        result.error_message = "Decryption failed: invalid padding or corrupted data";
        spdlog::warn("AES-CBC decryption: padding error");
    } catch (const Botan::Exception& e) {
        result.success = false;
        result.error_message = std::string("Botan error: ") + e.what();
        spdlog::error("AES-CBC decryption failed: {}", e.what());
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-CBC decryption failed: {}", e.what());
    }
    
    return result;
}

bool AES_CBC::is_suitable_for(core::SecurityLevel level) const {
    // CBC mode without authentication is not recommended for high security
    // Should be used with HMAC for integrity protection
    switch (level) {
        case core::SecurityLevel::WEAK:
        case core::SecurityLevel::MEDIUM:
            return true;
        case core::SecurityLevel::STRONG:
        case core::SecurityLevel::PARANOID:
            return false;  // Prefer AEAD modes for high security
        default:
            return false;
    }
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
