/**
 * @file test_aes_modes.cpp
 * @brief Tests for all AES modes: CFB, OFB, ECB, XTS
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "filevault/algorithms/symmetric/aes_cfb.hpp"
#include "filevault/algorithms/symmetric/aes_ofb.hpp"
#include "filevault/algorithms/symmetric/aes_ecb.hpp"
#include "filevault/algorithms/symmetric/aes_xts.hpp"
#include <vector>
#include <string>

using namespace filevault::algorithms::symmetric;
using namespace filevault::core;

// Test data
const std::string TEST_PLAINTEXT = "The quick brown fox jumps over the lazy dog. 1234567890!";
const std::vector<uint8_t> TEST_DATA(TEST_PLAINTEXT.begin(), TEST_PLAINTEXT.end());

// Helper to generate key of specific size
std::vector<uint8_t> generate_key(size_t size) {
    std::vector<uint8_t> key(size);
    for (size_t i = 0; i < size; ++i) {
        key[i] = static_cast<uint8_t>(i % 256);
    }
    return key;
}

// ============================================================================
// AES-CFB Tests
// ============================================================================

TEST_CASE("AES-CFB Basic Encrypt/Decrypt", "[aes-cfb]") {
    SECTION("AES-128-CFB round-trip") {
        AES_CFB cfb(128);
        auto key = generate_key(16);  // 128-bit key
        
        EncryptionConfig config;
        auto encrypted = cfb.encrypt(TEST_DATA, key, config);
        
        REQUIRE(encrypted.success);
        REQUIRE(!encrypted.data.empty());
        REQUIRE(encrypted.nonce.has_value());
        REQUIRE(encrypted.nonce.value().size() == 16);  // IV size
        
        // CFB is stream-like, output size = input size
        REQUIRE(encrypted.data.size() == TEST_DATA.size());
        
        // Decrypt
        config.nonce = encrypted.nonce;
        auto decrypted = cfb.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
    
    SECTION("AES-256-CFB round-trip") {
        AES_CFB cfb(256);
        auto key = generate_key(32);  // 256-bit key
        
        EncryptionConfig config;
        auto encrypted = cfb.encrypt(TEST_DATA, key, config);
        
        REQUIRE(encrypted.success);
        
        config.nonce = encrypted.nonce;
        auto decrypted = cfb.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
}

TEST_CASE("AES-CFB Different IVs produce different ciphertext", "[aes-cfb]") {
    AES_CFB cfb(256);
    auto key = generate_key(32);
    
    EncryptionConfig config;
    auto encrypted1 = cfb.encrypt(TEST_DATA, key, config);
    auto encrypted2 = cfb.encrypt(TEST_DATA, key, config);
    
    REQUIRE(encrypted1.success);
    REQUIRE(encrypted2.success);
    
    // Different IVs should produce different ciphertext
    REQUIRE(encrypted1.nonce.value() != encrypted2.nonce.value());
    REQUIRE(encrypted1.data != encrypted2.data);
}

// ============================================================================
// AES-OFB Tests
// ============================================================================

TEST_CASE("AES-OFB Basic Encrypt/Decrypt", "[aes-ofb]") {
    SECTION("AES-128-OFB round-trip") {
        AES_OFB ofb(128);
        auto key = generate_key(16);
        
        EncryptionConfig config;
        auto encrypted = ofb.encrypt(TEST_DATA, key, config);
        
        REQUIRE(encrypted.success);
        REQUIRE(!encrypted.data.empty());
        // OFB output size = input size
        REQUIRE(encrypted.data.size() == TEST_DATA.size());
        
        config.nonce = encrypted.nonce;
        auto decrypted = ofb.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
    
    SECTION("AES-256-OFB round-trip") {
        AES_OFB ofb(256);
        auto key = generate_key(32);
        
        EncryptionConfig config;
        auto encrypted = ofb.encrypt(TEST_DATA, key, config);
        
        REQUIRE(encrypted.success);
        
        config.nonce = encrypted.nonce;
        auto decrypted = ofb.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
}

// ============================================================================
// AES-ECB Tests (Note: ECB is INSECURE!)
// ============================================================================

TEST_CASE("AES-ECB Basic Encrypt/Decrypt", "[aes-ecb]") {
    SECTION("AES-128-ECB round-trip") {
        AES_ECB ecb(128);
        auto key = generate_key(16);
        
        EncryptionConfig config;
        auto encrypted = ecb.encrypt(TEST_DATA, key, config);
        
        INFO("ECB encrypt error: " << encrypted.error_message);
        REQUIRE(encrypted.success);
        REQUIRE(!encrypted.data.empty());
        // ECB uses PKCS7 padding, so output is larger
        REQUIRE(encrypted.data.size() >= TEST_DATA.size());
        REQUIRE(encrypted.data.size() % 16 == 0);  // Multiple of block size
        
        auto decrypted = ecb.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
    
    SECTION("AES-256-ECB round-trip") {
        AES_ECB ecb(256);
        auto key = generate_key(32);
        
        EncryptionConfig config;
        auto encrypted = ecb.encrypt(TEST_DATA, key, config);
        
        REQUIRE(encrypted.success);
        
        auto decrypted = ecb.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
}

TEST_CASE("AES-ECB Same plaintext produces same ciphertext (INSECURE!)", "[aes-ecb]") {
    AES_ECB ecb(256);
    auto key = generate_key(32);
    
    EncryptionConfig config;
    auto encrypted1 = ecb.encrypt(TEST_DATA, key, config);
    auto encrypted2 = ecb.encrypt(TEST_DATA, key, config);
    
    REQUIRE(encrypted1.success);
    REQUIRE(encrypted2.success);
    
    // ECB always produces same ciphertext for same plaintext - this is the weakness!
    REQUIRE(encrypted1.data == encrypted2.data);
}

TEST_CASE("AES-ECB security level is weak only", "[aes-ecb]") {
    AES_ECB ecb(256);
    
    REQUIRE(ecb.is_suitable_for(SecurityLevel::WEAK) == true);
    REQUIRE(ecb.is_suitable_for(SecurityLevel::MEDIUM) == false);
    REQUIRE(ecb.is_suitable_for(SecurityLevel::STRONG) == false);
    REQUIRE(ecb.is_suitable_for(SecurityLevel::PARANOID) == false);
}

// ============================================================================
// AES-XTS Tests (Disk encryption mode)
// ============================================================================

TEST_CASE("AES-XTS Basic Encrypt/Decrypt", "[aes-xts]") {
    SECTION("AES-128-XTS round-trip") {
        AES_XTS xts(128);
        // XTS uses double key (256 bits for AES-128-XTS)
        auto key = generate_key(32);  // 2 * 128/8 = 32 bytes
        
        REQUIRE(xts.key_size() == 32);
        
        // XTS needs at least 16 bytes
        std::vector<uint8_t> data = TEST_DATA;
        if (data.size() < 16) {
            data.resize(16, 0);
        }
        
        EncryptionConfig config;
        auto encrypted = xts.encrypt(data, key, config);
        
        REQUIRE(encrypted.success);
        REQUIRE(!encrypted.data.empty());
        // XTS doesn't expand data
        REQUIRE(encrypted.data.size() == data.size());
        
        config.nonce = encrypted.nonce;  // Tweak
        auto decrypted = xts.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == data);
    }
    
    SECTION("AES-256-XTS round-trip") {
        AES_XTS xts(256);
        // XTS uses double key (512 bits for AES-256-XTS)
        auto key = generate_key(64);  // 2 * 256/8 = 64 bytes
        
        REQUIRE(xts.key_size() == 64);
        
        EncryptionConfig config;
        auto encrypted = xts.encrypt(TEST_DATA, key, config);
        
        REQUIRE(encrypted.success);
        
        config.nonce = encrypted.nonce;
        auto decrypted = xts.decrypt(encrypted.data, key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
}

TEST_CASE("AES-XTS minimum block size", "[aes-xts]") {
    AES_XTS xts(256);
    auto key = generate_key(64);
    
    // Test with less than 16 bytes - should fail
    std::vector<uint8_t> small_data = {1, 2, 3, 4, 5};  // 5 bytes
    
    EncryptionConfig config;
    auto result = xts.encrypt(small_data, key, config);
    
    REQUIRE(!result.success);
    REQUIRE(result.error_message.find("16 bytes") != std::string::npos);
}

// ============================================================================
// Invalid Key Size Tests
// ============================================================================

TEST_CASE("AES modes reject invalid key sizes", "[aes][validation]") {
    SECTION("CFB rejects wrong key size") {
        AES_CFB cfb(256);
        auto wrong_key = generate_key(16);  // Should be 32
        
        EncryptionConfig config;
        auto result = cfb.encrypt(TEST_DATA, wrong_key, config);
        
        REQUIRE(!result.success);
        REQUIRE(result.error_message.find("key size") != std::string::npos);
    }
    
    SECTION("OFB rejects wrong key size") {
        AES_OFB ofb(256);
        auto wrong_key = generate_key(24);  // Should be 32
        
        EncryptionConfig config;
        auto result = ofb.encrypt(TEST_DATA, wrong_key, config);
        
        REQUIRE(!result.success);
    }
    
    SECTION("ECB rejects wrong key size") {
        AES_ECB ecb(128);
        auto wrong_key = generate_key(32);  // Should be 16
        
        EncryptionConfig config;
        auto result = ecb.encrypt(TEST_DATA, wrong_key, config);
        
        REQUIRE(!result.success);
    }
    
    SECTION("XTS rejects wrong key size") {
        AES_XTS xts(128);  // Needs 32-byte key (double)
        auto wrong_key = generate_key(16);  // Only 16 bytes
        
        EncryptionConfig config;
        auto result = xts.encrypt(TEST_DATA, wrong_key, config);
        
        REQUIRE(!result.success);
    }
}

// ============================================================================
// Decryption Without IV Tests
// ============================================================================

TEST_CASE("CFB/OFB decryption requires IV", "[aes][validation]") {
    SECTION("CFB decrypt without IV fails") {
        AES_CFB cfb(256);
        auto key = generate_key(32);
        
        EncryptionConfig config;
        auto encrypted = cfb.encrypt(TEST_DATA, key, config);
        REQUIRE(encrypted.success);
        
        // Try to decrypt without IV
        EncryptionConfig no_iv_config;
        auto result = cfb.decrypt(encrypted.data, key, no_iv_config);
        
        REQUIRE(!result.success);
        REQUIRE(result.error_message.find("IV") != std::string::npos);
    }
    
    SECTION("OFB decrypt without IV fails") {
        AES_OFB ofb(256);
        auto key = generate_key(32);
        
        EncryptionConfig config;
        auto encrypted = ofb.encrypt(TEST_DATA, key, config);
        REQUIRE(encrypted.success);
        
        EncryptionConfig no_iv_config;
        auto result = ofb.decrypt(encrypted.data, key, no_iv_config);
        
        REQUIRE(!result.success);
    }
    
    SECTION("XTS decrypt without tweak fails") {
        AES_XTS xts(256);
        auto key = generate_key(64);
        
        EncryptionConfig config;
        auto encrypted = xts.encrypt(TEST_DATA, key, config);
        REQUIRE(encrypted.success);
        
        EncryptionConfig no_tweak_config;
        auto result = xts.decrypt(encrypted.data, key, no_tweak_config);
        
        REQUIRE(!result.success);
        REQUIRE(result.error_message.find("Tweak") != std::string::npos);
    }
}
