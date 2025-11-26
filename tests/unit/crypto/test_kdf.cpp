#include <catch2/catch_test_macros.hpp>
#include "filevault/core/crypto_engine.hpp"
#include <algorithm>
#include <vector>
#include <string>

using namespace filevault::core;

TEST_CASE("KDF - Argon2id key derivation", "[kdf][argon2]") {
    CryptoEngine engine;
    engine.initialize();
    
    SECTION("Basic key derivation") {
        std::string password = "MySecretPassword123";
        std::vector<uint8_t> salt(32, 0xAB);
        
        EncryptionConfig config;
        config.kdf = KDFType::ARGON2ID;
        config.kdf_iterations = 3;  // Fast for testing
        config.kdf_memory_kb = 65536;  // 64MB
        
        auto key = engine.derive_key(password, salt, config);
        
        REQUIRE(key.size() == 32);  // 256-bit key
        REQUIRE_FALSE(std::all_of(key.begin(), key.end(), [](uint8_t b) { return b == 0; }));
    }
    
    SECTION("Same password and salt produce same key") {
        std::string password = "TestPassword";
        std::vector<uint8_t> salt(32, 0x01);
        
        EncryptionConfig config;
        config.kdf = KDFType::ARGON2ID;
        config.kdf_iterations = 3;  // Fast for testing
        config.kdf_memory_kb = 65536;
        
        auto key1 = engine.derive_key(password, salt, config);
        auto key2 = engine.derive_key(password, salt, config);
        
        REQUIRE(key1 == key2);
    }
    
    SECTION("Different salts produce different keys") {
        std::string password = "TestPassword";
        std::vector<uint8_t> salt1(32, 0x01);
        std::vector<uint8_t> salt2(32, 0x02);
        
        EncryptionConfig config;
        config.kdf = KDFType::ARGON2ID;
        config.kdf_iterations = 3;  // Fast for testing
        config.kdf_memory_kb = 65536;
        
        auto key1 = engine.derive_key(password, salt1, config);
        auto key2 = engine.derive_key(password, salt2, config);
        
        REQUIRE(key1 != key2);
    }
}

TEST_CASE("KDF - PBKDF2 key derivation", "[kdf][pbkdf2]") {
    CryptoEngine engine;
    engine.initialize();
    
    SECTION("PBKDF2-SHA256") {
        std::string password = "password123";
        std::vector<uint8_t> salt(32, 0xEF);
        
        EncryptionConfig config;
        config.kdf = KDFType::PBKDF2_SHA256;
        config.kdf_iterations = 10000;  // Reduced for testing speed
        
        auto key = engine.derive_key(password, salt, config);
        
        REQUIRE(key.size() == 32);
        REQUIRE_FALSE(std::all_of(key.begin(), key.end(), [](uint8_t b) { return b == 0; }));
    }
    
    SECTION("PBKDF2-SHA512") {
        std::string password = "StrongPassword!@#";
        std::vector<uint8_t> salt(32, 0x12);
        
        EncryptionConfig config;
        config.kdf = KDFType::PBKDF2_SHA512;
        config.kdf_iterations = 10000;  // Reduced for testing speed
        
        auto key = engine.derive_key(password, salt, config);
        
        REQUIRE(key.size() == 32);
    }
}

TEST_CASE("KDF - Iteration count impact", "[kdf][performance][.slow]") {  // Tagged .slow to skip by default
    CryptoEngine engine;
    engine.initialize();
    
    std::string password = "TestPassword";
    std::vector<uint8_t> salt(32, 0xAA);
    
    SECTION("Higher iterations take longer") {
        EncryptionConfig config1;
        config1.kdf = KDFType::PBKDF2_SHA256;
        config1.kdf_iterations = 1000;  // Reduced for fast testing
        
        EncryptionConfig config2;
        config2.kdf = KDFType::PBKDF2_SHA256;
        config2.kdf_iterations = 5000;  // Reduced for fast testing
        
        auto start1 = std::chrono::high_resolution_clock::now();
        auto key1 = engine.derive_key(password, salt, config1);
        auto end1 = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
        
        auto start2 = std::chrono::high_resolution_clock::now();
        auto key2 = engine.derive_key(password, salt, config2);
        auto end2 = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
        
        // 10x iterations should take significantly longer
        REQUIRE(duration2 > duration1);
    }
}
