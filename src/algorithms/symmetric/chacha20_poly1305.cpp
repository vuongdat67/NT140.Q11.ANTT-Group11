#include "filevault/algorithms/symmetric/chacha20_poly1305.hpp"
#include <botan/auto_rng.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

ChaCha20Poly1305::ChaCha20Poly1305() {
    spdlog::debug("Created ChaCha20-Poly1305 cipher");
}

std::string ChaCha20Poly1305::name() const {
    return "ChaCha20-Poly1305";
}

core::AlgorithmType ChaCha20Poly1305::type() const {
    return core::AlgorithmType::CHACHA20_POLY1305;
}

core::CryptoResult ChaCha20Poly1305::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) {
    
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate key size
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size (must be 32 bytes for ChaCha20)";
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
        // Botan cipher name for ChaCha20-Poly1305 (IETF variant)
        auto cipher = Botan::AEAD_Mode::create("ChaCha20Poly1305", Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create ChaCha20-Poly1305 cipher";
            return result;
        }
        
        // Set key
        cipher->set_key(key.data(), key.size());
        
        // Set associated data if provided
        if (config.associated_data.has_value() && !config.associated_data.value().empty()) {
            const auto& ad = config.associated_data.value();
            cipher->set_associated_data(ad.data(), ad.size());
        }
        
        // Start encryption with nonce
        cipher->start(nonce.data(), nonce.size());
        
        // Prepare output buffer (plaintext will be encrypted in place, tag appended)
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
        result.algorithm_used = type();
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("Encrypted {} bytes -> {} bytes + {} byte tag in {:.2f}ms using ChaCha20-Poly1305",
                     plaintext.size(), result.data.size(), tag_size(), result.processing_time_ms);
        
        return result;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("ChaCha20-Poly1305 encryption failed: ") + e.what();
        return result;
    }
}

core::CryptoResult ChaCha20Poly1305::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) {
    
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Validate key size
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size (must be 32 bytes for ChaCha20)";
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
            result.error_message = "Invalid nonce size (must be 12 bytes)";
            return result;
        }
        
        if (tag.size() != tag_size()) {
            result.success = false;
            result.error_message = "Invalid tag size (must be 16 bytes)";
            return result;
        }
        
        // Create AEAD cipher
        auto cipher = Botan::AEAD_Mode::create("ChaCha20Poly1305", Botan::Cipher_Dir::Decryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create ChaCha20-Poly1305 cipher";
            return result;
        }
        
        // Set key
        cipher->set_key(key.data(), key.size());
        
        // Set associated data if provided
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
        
        // Decrypt and verify authentication tag
        cipher->finish(buffer);
        
        // Store plaintext
        result.data.assign(buffer.begin(), buffer.end());
        
        // Fill metadata
        result.success = true;
        result.algorithm_used = type();
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("Decrypted {} bytes -> {} bytes in {:.2f}ms using ChaCha20-Poly1305",
                     ciphertext.size(), result.data.size(), result.processing_time_ms);
        
        return result;
        
    } catch (const Botan::Invalid_Authentication_Tag&) {
        result.success = false;
        result.error_message = "Authentication failed: Invalid tag (data may be corrupted or tampered)";
        return result;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("ChaCha20-Poly1305 decryption failed: ") + e.what();
        return result;
    }
}

bool ChaCha20Poly1305::is_suitable_for(core::SecurityLevel level) const {
    // ChaCha20-Poly1305 is suitable for all security levels
    // It's a modern, secure AEAD cipher recommended by IETF
    // Particularly good for platforms without AES-NI hardware
    (void)level; // Unused
    return true;
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
