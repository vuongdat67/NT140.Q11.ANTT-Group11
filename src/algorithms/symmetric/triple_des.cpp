/**
 * @file triple_des.cpp
 * @brief Triple-DES (3DES) encryption implementation
 */

#include "filevault/algorithms/symmetric/triple_des.hpp"
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

TripleDES::TripleDES() {
    spdlog::debug("Created Triple-DES algorithm");
    spdlog::debug("Triple-DES is a legacy algorithm. Consider using AES instead.");
}

core::CryptoResult TripleDES::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate key size (24 bytes for 3DES)
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size. 3DES requires 24-byte key, got " + 
                std::to_string(key.size());
            return result;
        }
        
        // Generate or use provided IV
        std::vector<uint8_t> iv;
        if (config.nonce.has_value() && config.nonce.value().size() == iv_size()) {
            iv = config.nonce.value();
            spdlog::debug("3DES: Using provided IV");
        } else {
            Botan::AutoSeeded_RNG rng;
            iv.resize(iv_size());
            rng.randomize(iv.data(), iv.size());
            spdlog::debug("3DES: Generated new IV ({} bytes)", iv.size());
        }
        
        // Create cipher
        auto cipher = Botan::Cipher_Mode::create("TripleDES/CBC/PKCS7", Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create 3DES cipher";
            return result;
        }
        
        // Set key and IV
        cipher->set_key(key.data(), key.size());
        cipher->start(iv);
        
        // Encrypt
        Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
        cipher->finish(buffer);
        
        // Store result
        result.data.assign(buffer.begin(), buffer.end());
        result.nonce = std::move(iv);
        result.success = true;
        result.algorithm_used = type();
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("3DES encryption: {} bytes -> {} bytes in {:.2f}ms",
                      plaintext.size(), result.data.size(), result.processing_time_ms);
        
    } catch (const Botan::Exception& e) {
        result.success = false;
        result.error_message = std::string("Botan error: ") + e.what();
        spdlog::error("3DES encryption failed: {}", e.what());
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("3DES encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult TripleDES::decrypt(
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
            result.error_message = "Invalid key size. 3DES requires 24-byte key, got " + 
                std::to_string(key.size());
            return result;
        }
        
        // Get IV from config
        if (!config.nonce.has_value() || config.nonce.value().size() != iv_size()) {
            result.success = false;
            result.error_message = "IV must be provided for 3DES decryption (8 bytes)";
            return result;
        }
        
        auto& iv = config.nonce.value();
        
        // Validate ciphertext
        if (ciphertext.size() == 0 || ciphertext.size() % block_size() != 0) {
            result.success = false;
            result.error_message = "Invalid ciphertext size (must be multiple of 8)";
            return result;
        }
        
        // Create cipher
        auto cipher = Botan::Cipher_Mode::create("TripleDES/CBC/PKCS7", Botan::Cipher_Dir::Decryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create 3DES cipher";
            return result;
        }
        
        // Set key and IV
        cipher->set_key(key.data(), key.size());
        cipher->start(iv);
        
        // Decrypt
        Botan::secure_vector<uint8_t> buffer(ciphertext.begin(), ciphertext.end());
        cipher->finish(buffer);
        
        // Store result
        result.data.assign(buffer.begin(), buffer.end());
        result.success = true;
        result.algorithm_used = type();
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("3DES decryption: {} bytes -> {} bytes in {:.2f}ms",
                      ciphertext.size(), result.data.size(), result.processing_time_ms);
        
    } catch (const Botan::Decoding_Error&) {
        result.success = false;
        result.error_message = "Decryption failed: invalid padding or corrupted data";
        spdlog::warn("3DES decryption: padding error");
    } catch (const Botan::Exception& e) {
        result.success = false;
        result.error_message = std::string("Botan error: ") + e.what();
        spdlog::error("3DES decryption failed: {}", e.what());
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("3DES decryption failed: {}", e.what());
    }
    
    return result;
}

bool TripleDES::is_suitable_for(core::SecurityLevel level) const {
    // 3DES has 112-bit effective security - only for legacy/compatibility
    switch (level) {
        case core::SecurityLevel::WEAK:
            return true;
        case core::SecurityLevel::MEDIUM:
        case core::SecurityLevel::STRONG:
        case core::SecurityLevel::PARANOID:
            return false;  // Use AES instead
        default:
            return false;
    }
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
