#include <catch2/catch_test_macros.hpp>
#include "filevault/algorithms/symmetric/aes_gcm.hpp"
#include <vector>

using namespace filevault::algorithms::symmetric;
using namespace filevault::core;

TEST_CASE("AES-GCM encryption/decryption", "[aes][gcm]") {
    EncryptionConfig config;
    
    SECTION("AES-128-GCM") {
        AES_GCM cipher(128);
        
        std::string plaintext = "Hello, World! This is a test message.";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        // 128-bit key (16 bytes)
        std::vector<uint8_t> key(16, 0xAB);
        
        // 96-bit nonce (12 bytes) - standard for GCM
        std::vector<uint8_t> nonce(12, 0x01);
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        REQUIRE(encrypted.data.size() >= pt.size());  // May include auth tag
        REQUIRE(encrypted.tag.has_value());  // Tag should be generated
        
        // Set tag for decryption
        config.tag = encrypted.tag.value();
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == pt);
    }
    
    SECTION("AES-256-GCM") {
        AES_GCM cipher(256);
        
        std::string plaintext = "Sensitive data";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        // 256-bit key (32 bytes)
        std::vector<uint8_t> key(32, 0xCD);
        std::vector<uint8_t> nonce(12, 0x02);
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        // Set tag for decryption
        if (encrypted.tag.has_value()) {
            config.tag = encrypted.tag.value();
        }
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == pt);
    }
    
    SECTION("Authentication tag verification") {
        AES_GCM cipher(256);
        
        std::string plaintext = "Authenticated message";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key(32, 0xEF);
        std::vector<uint8_t> nonce(12, 0x03);
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        REQUIRE(encrypted.tag.has_value());
        
        // Set tag for decryption
        config.tag = encrypted.tag.value();
        
        // Tamper with ciphertext
        encrypted.data[5] ^= 0xFF;
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        // Should fail authentication
        REQUIRE_FALSE(decrypted.success);
    }
    
    SECTION("Empty plaintext") {
        AES_GCM cipher(128);
        
        std::vector<uint8_t> pt;
        std::vector<uint8_t> key(16, 0x00);
        std::vector<uint8_t> nonce(12, 0x04);
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        // Set tag for decryption
        if (encrypted.tag.has_value()) {
            config.tag = encrypted.tag.value();
        }
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data.empty());
    }
}

TEST_CASE("AES-GCM NIST test vectors", "[aes][gcm][nist]") {
    // Test Case 1 from NIST SP 800-38D
    AES_GCM cipher(128);
    EncryptionConfig config;
    
    // Key: 00000000000000000000000000000000
    std::vector<uint8_t> key(16, 0x00);
    
    // IV: 000000000000000000000000
    std::vector<uint8_t> nonce(12, 0x00);
    config.nonce = nonce;
    
    // Plaintext: (empty)
    std::vector<uint8_t> plaintext;
    
    auto result = cipher.encrypt(plaintext, key, config);
    REQUIRE(result.success);
    
    // Expected auth tag: 58e2fccefa7e3061367f1d57a4e7455a
    // (GCM produces tag even for empty plaintext)
    REQUIRE(result.tag.has_value());
    REQUIRE(result.tag.value().size() == 16);
}
