/**
 * @file aes_xts.cpp
 * @brief AES-XTS encryption implementation for disk encryption
 */

#include "filevault/algorithms/symmetric/aes_xts.hpp"
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

AES_XTS::AES_XTS(size_t key_bits) : key_bits_(key_bits) {
    if (key_bits != 128 && key_bits != 256) {
        throw std::invalid_argument("AES-XTS key size must be 128 or 256 bits (per cipher)");
    }
    
    switch (key_bits) {
        case 128:
            type_ = core::AlgorithmType::AES_128_XTS;
            botan_name_ = "AES-128/XTS";
            break;
        case 256:
        default:
            type_ = core::AlgorithmType::AES_256_XTS;
            botan_name_ = "AES-256/XTS";
            break;
    }
    
    spdlog::debug("Created AES-{}-XTS algorithm (total key: {} bits)", 
                  key_bits, key_bits * 2);
}

std::string AES_XTS::name() const {
    return "AES-" + std::to_string(key_bits_) + "-XTS";
}

core::AlgorithmType AES_XTS::type() const {
    return type_;
}

core::CryptoResult AES_XTS::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // XTS requires double key size
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size. Expected " + 
                std::to_string(key_size()) + " bytes, got " + 
                std::to_string(key.size());
            return result;
        }
        
        // XTS requires at least one block
        if (plaintext.size() < block_size()) {
            result.success = false;
            result.error_message = "Plaintext must be at least 16 bytes for XTS mode";
            return result;
        }
        
        // Generate or use provided tweak
        std::vector<uint8_t> tweak;
        if (config.nonce.has_value() && config.nonce.value().size() == tweak_size()) {
            tweak = config.nonce.value();
        } else {
            Botan::AutoSeeded_RNG rng;
            tweak.resize(tweak_size());
            rng.randomize(tweak.data(), tweak.size());
        }
        
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-XTS cipher";
            return result;
        }
        
        cipher->set_key(key.data(), key.size());
        cipher->start(tweak);
        
        Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
        cipher->finish(buffer);
        
        result.data.assign(buffer.begin(), buffer.end());
        result.nonce = std::move(tweak);
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-XTS encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult AES_XTS::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size";
            return result;
        }
        
        if (!config.nonce.has_value() || config.nonce.value().size() != tweak_size()) {
            result.success = false;
            result.error_message = "Tweak must be provided for XTS decryption (16 bytes)";
            return result;
        }
        
        if (ciphertext.size() < block_size()) {
            result.success = false;
            result.error_message = "Ciphertext must be at least 16 bytes for XTS mode";
            return result;
        }
        
        auto& tweak = config.nonce.value();
        
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Decryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-XTS cipher";
            return result;
        }
        
        cipher->set_key(key.data(), key.size());
        cipher->start(tweak);
        
        Botan::secure_vector<uint8_t> buffer(ciphertext.begin(), ciphertext.end());
        cipher->finish(buffer);
        
        result.data.assign(buffer.begin(), buffer.end());
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-XTS decryption failed: {}", e.what());
    }
    
    return result;
}

bool AES_XTS::is_suitable_for(core::SecurityLevel level) const {
    // XTS is designed for disk encryption, suitable for medium-high security
    switch (level) {
        case core::SecurityLevel::WEAK:
        case core::SecurityLevel::MEDIUM:
        case core::SecurityLevel::STRONG:
            return true;
        case core::SecurityLevel::PARANOID:
            return false;  // Prefer authenticated modes
        default:
            return false;
    }
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
