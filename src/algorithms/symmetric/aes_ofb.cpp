/**
 * @file aes_ofb.cpp
 * @brief AES-OFB encryption implementation
 */

#include "filevault/algorithms/symmetric/aes_ofb.hpp"
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

AES_OFB::AES_OFB(size_t key_bits) : key_bits_(key_bits) {
    if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        throw std::invalid_argument("AES-OFB key size must be 128, 192, or 256 bits");
    }
    
    switch (key_bits) {
        case 128:
            type_ = core::AlgorithmType::AES_128_OFB;
            botan_name_ = "AES-128/OFB";
            break;
        case 192:
            type_ = core::AlgorithmType::AES_192_OFB;
            botan_name_ = "AES-192/OFB";
            break;
        case 256:
        default:
            type_ = core::AlgorithmType::AES_256_OFB;
            botan_name_ = "AES-256/OFB";
            break;
    }
    
    spdlog::debug("Created AES-{}-OFB algorithm", key_bits);
}

std::string AES_OFB::name() const {
    return "AES-" + std::to_string(key_bits_) + "-OFB";
}

core::AlgorithmType AES_OFB::type() const {
    return type_;
}

core::CryptoResult AES_OFB::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
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
        } else {
            Botan::AutoSeeded_RNG rng;
            iv.resize(iv_size());
            rng.randomize(iv.data(), iv.size());
        }
        
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Encryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-OFB cipher";
            return result;
        }
        
        cipher->set_key(key.data(), key.size());
        cipher->start(iv);
        
        Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
        cipher->finish(buffer);
        
        result.data.assign(buffer.begin(), buffer.end());
        result.nonce = std::move(iv);
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-OFB encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult AES_OFB::decrypt(
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
        
        if (!config.nonce.has_value() || config.nonce.value().size() != iv_size()) {
            result.success = false;
            result.error_message = "IV must be provided for OFB decryption (16 bytes)";
            return result;
        }
        
        auto& iv = config.nonce.value();
        
        auto cipher = Botan::Cipher_Mode::create(botan_name_, Botan::Cipher_Dir::Decryption);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES-OFB cipher";
            return result;
        }
        
        cipher->set_key(key.data(), key.size());
        cipher->start(iv);
        
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
        spdlog::error("AES-OFB decryption failed: {}", e.what());
    }
    
    return result;
}

bool AES_OFB::is_suitable_for(core::SecurityLevel level) const {
    switch (level) {
        case core::SecurityLevel::WEAK:
        case core::SecurityLevel::MEDIUM:
            return true;
        case core::SecurityLevel::STRONG:
        case core::SecurityLevel::PARANOID:
            return false;
        default:
            return false;
    }
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
