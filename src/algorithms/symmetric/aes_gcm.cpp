#include "filevault/algorithms/symmetric/aes_gcm.hpp"
#include <botan/auto_rng.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

AES_GCM::AES_GCM(size_t key_bits) 
    : key_bits_(key_bits) {
    
    // Determine algorithm type and Botan cipher name
    switch (key_bits_) {
        case 128:
            type_ = core::AlgorithmType::AES_128_GCM;
            botan_name_ = "AES-128/GCM";
            break;
        case 192:
            type_ = core::AlgorithmType::AES_192_GCM;
            botan_name_ = "AES-192/GCM";
            break;
        case 256:
            type_ = core::AlgorithmType::AES_256_GCM;
            botan_name_ = "AES-256/GCM";
            break;
        default:
            throw std::invalid_argument("Invalid AES key size. Must be 128, 192, or 256 bits");
    }
    
    spdlog::debug("Created {} cipher", name());
}

std::string AES_GCM::name() const {
    return botan_name_;
}

core::AlgorithmType AES_GCM::type() const {
    return type_;
}

core::CryptoResult AES_GCM::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) {
    
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate inputs
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size";
            return result;
        }
        
        // Generate or use provided nonce (SECURITY FIX: auto-generate unique nonce)
        std::vector<uint8_t> nonce;
        if (config.nonce.has_value() && config.nonce.value().size() == nonce_size()) {
            // Allow override for testing only
            nonce = config.nonce.value();
            spdlog::debug("Using provided nonce (testing mode)");
        } else {
            // CRITICAL: Generate NEW unique nonce for THIS encryption
            Botan::AutoSeeded_RNG rng;
            nonce.resize(nonce_size());
            rng.randomize(nonce.data(), nonce.size());
            spdlog::debug("Generated new unique nonce ({} bytes)", nonce.size());
        }
        
        // Create AEAD cipher
        auto cipher = Botan::AEAD_Mode::create(botan_name_, Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AEAD cipher";
            return result;
        }
        
        // Set key
        cipher->set_key(key.data(), key.size());
        
        // Set associated data if provided in config
        if (config.associated_data.has_value() && !config.associated_data.value().empty()) {
            const auto& ad = config.associated_data.value();
            cipher->set_associated_data(ad.data(), ad.size());
        }
        
        // Start encryption with nonce
        cipher->start(nonce.data(), nonce.size());
        
        // Prepare output buffer (ciphertext + tag)
        Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
        cipher->finish(buffer);
        
        // Extract tag (last 16 bytes)
        if (buffer.size() < tag_size()) {
            result.success = false;
            result.error_message = "Invalid ciphertext size";
            return result;
        }
        
        size_t ciphertext_len = buffer.size() - tag_size();
        
        // Store ciphertext (without tag)
        result.data.assign(buffer.begin(), buffer.begin() + ciphertext_len);
        
        // Store tag separately
        result.tag = std::vector<uint8_t>(
            buffer.begin() + ciphertext_len,
            buffer.end()
        );
        
        // Store nonce
        result.nonce = std::vector<uint8_t>(nonce.begin(), nonce.end());
        
        // Fill metadata
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("Encrypted {} bytes -> {} bytes + {} byte tag in {:.2f}ms",
                     plaintext.size(), result.data.size(), tag_size(), result.processing_time_ms);
        
        return result;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Encryption failed: ") + e.what();
        return result;
    }
}

core::CryptoResult AES_GCM::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) {
    
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate inputs
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size";
            return result;
        }
        
        // Get nonce and tag from config
        if (!config.nonce.has_value() || !config.tag.has_value()) {
            result.success = false;
            result.error_message = "Nonce and tag must be provided in config";
            return result;
        }
        
        auto& nonce = config.nonce.value();
        auto& tag = config.tag.value();
        
        if (nonce.size() != nonce_size()) {
            result.success = false;
            result.error_message = "Invalid nonce size";
            return result;
        }
        
        if (tag.size() != tag_size()) {
            result.success = false;
            result.error_message = "Invalid tag size";
            return result;
        }
        
        // Create AEAD cipher
        auto cipher = Botan::AEAD_Mode::create(botan_name_, Botan::Cipher_Dir::Decryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AEAD cipher";
            return result;
        }
        
        // Set key
        cipher->set_key(key.data(), key.size());
        
        // Set associated data if provided in config
        if (config.associated_data.has_value() && !config.associated_data.value().empty()) {
            const auto& ad = config.associated_data.value();
            cipher->set_associated_data(ad.data(), ad.size());
        }
        
        // Start decryption with nonce
        cipher->start(nonce.data(), nonce.size());
        
        // Combine ciphertext + tag
        Botan::secure_vector<uint8_t> buffer;
        buffer.reserve(ciphertext.size() + tag.size());
        buffer.insert(buffer.end(), ciphertext.begin(), ciphertext.end());
        buffer.insert(buffer.end(), tag.begin(), tag.end());
        
        // Decrypt and verify
        cipher->finish(buffer);
        
        // Store plaintext
        result.data.assign(buffer.begin(), buffer.end());
        
        // Fill metadata
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("Decrypted {} bytes -> {} bytes in {:.2f}ms",
                     ciphertext.size(), result.data.size(), result.processing_time_ms);
        
        return result;
        
    } catch (const Botan::Invalid_Authentication_Tag&) {
        result.success = false;
        result.error_message = "Authentication failed: Invalid tag (data may be corrupted or tampered)";
        return result;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Decryption failed: ") + e.what();
        return result;
    }
}

bool AES_GCM::is_suitable_for(core::SecurityLevel level) const {
    // AES-GCM is suitable for all security levels
    // The security comes from key length and KDF parameters
    (void)level; // Unused
    return true;
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
