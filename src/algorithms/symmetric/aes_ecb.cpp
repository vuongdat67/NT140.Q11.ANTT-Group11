/**
 * @file aes_ecb.cpp
 * @brief AES-ECB encryption implementation
 * 
 * ⚠️ WARNING: ECB mode is NOT secure for most uses!
 * 
 * Note: Botan 3.x doesn't expose ECB through Cipher_Mode, so we use
 * BlockCipher directly with manual PKCS7 padding.
 */

#include "filevault/algorithms/symmetric/aes_ecb.hpp"
#include <botan/block_cipher.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace symmetric {

AES_ECB::AES_ECB(size_t key_bits) : key_bits_(key_bits) {
    if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        throw std::invalid_argument("AES-ECB key size must be 128, 192, or 256 bits");
    }
    
    switch (key_bits) {
        case 128:
            type_ = core::AlgorithmType::AES_128_ECB;
            botan_name_ = "AES-128";
            break;
        case 192:
            type_ = core::AlgorithmType::AES_192_ECB;
            botan_name_ = "AES-192";
            break;
        case 256:
        default:
            type_ = core::AlgorithmType::AES_256_ECB;
            botan_name_ = "AES-256";
            break;
    }
    
    spdlog::warn("Created AES-{}-ECB algorithm - ECB mode is insecure!", key_bits);
}

// PKCS7 padding
static std::vector<uint8_t> add_pkcs7_padding(std::span<const uint8_t> data, size_t block_size) {
    size_t padding_len = block_size - (data.size() % block_size);
    std::vector<uint8_t> padded(data.begin(), data.end());
    padded.resize(data.size() + padding_len, static_cast<uint8_t>(padding_len));
    return padded;
}

static std::vector<uint8_t> remove_pkcs7_padding(std::span<const uint8_t> data) {
    if (data.empty()) {
        throw std::runtime_error("Empty data");
    }
    uint8_t padding_len = data.back();
    if (padding_len == 0 || padding_len > 16 || padding_len > data.size()) {
        throw std::runtime_error("Invalid padding length");
    }
    // Verify padding
    for (size_t i = data.size() - padding_len; i < data.size(); ++i) {
        if (data[i] != padding_len) {
            throw std::runtime_error("Invalid padding bytes");
        }
    }
    return std::vector<uint8_t>(data.begin(), data.end() - padding_len);
}

std::string AES_ECB::name() const {
    return "AES-" + std::to_string(key_bits_) + "-ECB";
}

core::AlgorithmType AES_ECB::type() const {
    return type_;
}

core::CryptoResult AES_ECB::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    [[maybe_unused]] const core::EncryptionConfig& config
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
        
        spdlog::warn("Using ECB mode - this is insecure for most applications!");
        
        // Create block cipher (not Cipher_Mode - ECB not exposed in Botan 3)
        auto cipher = Botan::BlockCipher::create(botan_name_);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES block cipher";
            return result;
        }
        
        cipher->set_key(key.data(), key.size());
        
        // Add PKCS7 padding
        std::vector<uint8_t> padded = add_pkcs7_padding(plaintext, block_size());
        
        // Encrypt each block
        std::vector<uint8_t> ciphertext(padded.size());
        for (size_t i = 0; i < padded.size(); i += block_size()) {
            cipher->encrypt(&padded[i], &ciphertext[i]);
        }
        
        result.data = std::move(ciphertext);
        // ECB doesn't use nonce/IV - set empty to indicate this
        result.nonce = std::vector<uint8_t>();
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-ECB encryption failed: {}", e.what());
    }
    
    return result;
}

core::CryptoResult AES_ECB::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    [[maybe_unused]] const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        if (key.size() != key_size()) {
            result.success = false;
            result.error_message = "Invalid key size";
            return result;
        }
        
        if (ciphertext.size() == 0 || ciphertext.size() % block_size() != 0) {
            result.success = false;
            result.error_message = "Invalid ciphertext size (must be multiple of 16)";
            return result;
        }
        
        // Create block cipher (not Cipher_Mode - ECB not exposed in Botan 3)
        auto cipher = Botan::BlockCipher::create(botan_name_);
        if (!cipher) {
            result.success = false;
            result.error_message = "Failed to create AES block cipher";
            return result;
        }
        
        cipher->set_key(key.data(), key.size());
        
        // Decrypt each block
        std::vector<uint8_t> decrypted(ciphertext.size());
        for (size_t i = 0; i < ciphertext.size(); i += block_size()) {
            cipher->decrypt(&ciphertext[i], &decrypted[i]);
        }
        
        // Remove PKCS7 padding
        result.data = remove_pkcs7_padding(decrypted);
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
    } catch (const std::runtime_error& e) {
        result.success = false;
        result.error_message = "Decryption failed: invalid padding or corrupted data";
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error: ") + e.what();
        spdlog::error("AES-ECB decryption failed: {}", e.what());
    }
    
    return result;
}

bool AES_ECB::is_suitable_for(core::SecurityLevel level) const {
    // ECB is only suitable for WEAK security level
    return level == core::SecurityLevel::WEAK;
}

} // namespace symmetric
} // namespace algorithms
} // namespace filevault
