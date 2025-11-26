/**
 * @file test_rsa.cpp
 * @brief Tests for RSA asymmetric encryption and signatures
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "filevault/algorithms/asymmetric/rsa.hpp"
#include <vector>
#include <string>

using namespace filevault::algorithms::asymmetric;
using namespace filevault::core;

// Test data - keep small for RSA (limited by key size)
const std::string TEST_MESSAGE = "Hello, RSA encryption!";
const std::vector<uint8_t> TEST_DATA(TEST_MESSAGE.begin(), TEST_MESSAGE.end());

// ============================================================================
// RSA Key Generation Tests
// ============================================================================

TEST_CASE("RSA Key Pair Generation", "[rsa][keygen]") {
    SECTION("Generate 2048-bit key pair") {
        RSA rsa(2048);
        auto key_pair = rsa.generate_key_pair();
        
        REQUIRE(!key_pair.public_key.empty());
        REQUIRE(!key_pair.private_key.empty());
        REQUIRE(key_pair.bits == 2048);
        
        // Check PEM format markers
        std::string pub_pem(key_pair.public_key.begin(), key_pair.public_key.end());
        std::string priv_pem(key_pair.private_key.begin(), key_pair.private_key.end());
        
        REQUIRE(pub_pem.find("-----BEGIN PUBLIC KEY-----") != std::string::npos);
        REQUIRE(priv_pem.find("-----BEGIN PRIVATE KEY-----") != std::string::npos);
    }
    
    SECTION("Generate 4096-bit key pair") {
        RSA rsa(4096);
        auto key_pair = rsa.generate_key_pair();
        
        REQUIRE(!key_pair.public_key.empty());
        REQUIRE(!key_pair.private_key.empty());
        REQUIRE(key_pair.bits == 4096);
    }
}

// ============================================================================
// RSA Encryption/Decryption Tests
// ============================================================================

TEST_CASE("RSA Basic Encrypt/Decrypt", "[rsa][crypto]") {
    SECTION("RSA-2048 round-trip") {
        RSA rsa(2048);
        auto key_pair = rsa.generate_key_pair();
        
        EncryptionConfig config;
        
        // Encrypt with public key
        auto encrypted = rsa.encrypt(TEST_DATA, key_pair.public_key, config);
        
        REQUIRE(encrypted.success);
        REQUIRE(!encrypted.data.empty());
        REQUIRE(encrypted.data != TEST_DATA);  // Should be different
        
        // Decrypt with private key
        auto decrypted = rsa.decrypt(encrypted.data, key_pair.private_key, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
    
    SECTION("RSA-4096 round-trip") {
        RSA rsa(4096);
        auto key_pair = rsa.generate_key_pair();
        
        EncryptionConfig config;
        
        auto encrypted = rsa.encrypt(TEST_DATA, key_pair.public_key, config);
        REQUIRE(encrypted.success);
        
        auto decrypted = rsa.decrypt(encrypted.data, key_pair.private_key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == TEST_DATA);
    }
}

TEST_CASE("RSA Maximum Plaintext Size", "[rsa][limits]") {
    RSA rsa(2048);
    auto key_pair = rsa.generate_key_pair();
    
    SECTION("Accept data within limit") {
        // RSA-2048 with OAEP SHA-256: max = 256 - 66 = 190 bytes
        size_t max_size = rsa.max_plaintext_size();
        REQUIRE(max_size == 190);
        
        std::vector<uint8_t> max_data(max_size, 'X');
        
        EncryptionConfig config;
        auto result = rsa.encrypt(max_data, key_pair.public_key, config);
        
        REQUIRE(result.success);
    }
    
    SECTION("Reject data exceeding limit") {
        size_t too_large = rsa.max_plaintext_size() + 1;
        std::vector<uint8_t> large_data(too_large, 'X');
        
        EncryptionConfig config;
        auto result = rsa.encrypt(large_data, key_pair.public_key, config);
        
        REQUIRE(!result.success);
        REQUIRE(result.error_message.find("too large") != std::string::npos);
    }
}

TEST_CASE("RSA Wrong Key Decryption", "[rsa][security]") {
    RSA rsa(2048);
    auto key_pair1 = rsa.generate_key_pair();
    auto key_pair2 = rsa.generate_key_pair();
    
    EncryptionConfig config;
    
    // Encrypt with key_pair1's public key
    auto encrypted = rsa.encrypt(TEST_DATA, key_pair1.public_key, config);
    REQUIRE(encrypted.success);
    
    // Try to decrypt with key_pair2's private key - should fail
    auto decrypted = rsa.decrypt(encrypted.data, key_pair2.private_key, config);
    
    REQUIRE(!decrypted.success);
}

// ============================================================================
// RSA Signature Tests
// ============================================================================

TEST_CASE("RSA Sign and Verify", "[rsa][signature]") {
    RSA rsa(2048);
    auto key_pair = rsa.generate_key_pair();
    
    SECTION("Valid signature verifies") {
        auto signature = rsa.sign(TEST_DATA, key_pair.private_key);
        
        REQUIRE(!signature.empty());
        
        bool valid = rsa.verify(TEST_DATA, signature, key_pair.public_key);
        REQUIRE(valid);
    }
    
    SECTION("Modified data fails verification") {
        auto signature = rsa.sign(TEST_DATA, key_pair.private_key);
        
        // Modify the data
        std::vector<uint8_t> modified_data = TEST_DATA;
        modified_data[0] ^= 0xFF;
        
        bool valid = rsa.verify(modified_data, signature, key_pair.public_key);
        REQUIRE(!valid);
    }
    
    SECTION("Wrong key fails verification") {
        auto key_pair2 = rsa.generate_key_pair();
        
        auto signature = rsa.sign(TEST_DATA, key_pair.private_key);
        
        // Verify with different public key
        bool valid = rsa.verify(TEST_DATA, signature, key_pair2.public_key);
        REQUIRE(!valid);
    }
}

// ============================================================================
// RSA Security Level Tests
// ============================================================================

TEST_CASE("RSA Security Level Suitability", "[rsa][security-level]") {
    SECTION("RSA-2048 security levels") {
        RSA rsa(2048);
        
        REQUIRE(rsa.is_suitable_for(SecurityLevel::WEAK) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::MEDIUM) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::STRONG) == false);  // Needs 3072+
        REQUIRE(rsa.is_suitable_for(SecurityLevel::PARANOID) == false);
    }
    
    SECTION("RSA-3072 security levels") {
        RSA rsa(3072);
        
        REQUIRE(rsa.is_suitable_for(SecurityLevel::WEAK) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::MEDIUM) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::STRONG) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::PARANOID) == false);  // Needs 4096
    }
    
    SECTION("RSA-4096 security levels") {
        RSA rsa(4096);
        
        REQUIRE(rsa.is_suitable_for(SecurityLevel::WEAK) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::MEDIUM) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::STRONG) == true);
        REQUIRE(rsa.is_suitable_for(SecurityLevel::PARANOID) == true);
    }
}

// ============================================================================
// RSA Algorithm Properties Tests
// ============================================================================

TEST_CASE("RSA Algorithm Properties", "[rsa][properties]") {
    SECTION("RSA-2048 properties") {
        RSA rsa(2048);
        
        REQUIRE(rsa.name() == "RSA-2048");
        REQUIRE(rsa.type() == AlgorithmType::RSA_2048);
        REQUIRE(rsa.key_size() == 256);  // 2048/8
        REQUIRE(rsa.requires_padding() == true);
        REQUIRE(rsa.is_authenticated() == false);
    }
    
    SECTION("RSA-4096 properties") {
        RSA rsa(4096);
        
        REQUIRE(rsa.name() == "RSA-4096");
        REQUIRE(rsa.type() == AlgorithmType::RSA_4096);
        REQUIRE(rsa.key_size() == 512);  // 4096/8
    }
}
