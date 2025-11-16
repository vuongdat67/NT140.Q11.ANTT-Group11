#include <catch2/catch_test_macros.hpp>
#include "filevault/core/crypto_engine.hpp"
#include "filevault/compression/compressor.hpp"
#include <vector>
#include <string>

using namespace filevault::core;
using filevault::compression::CompressionService;
using CompressionType = filevault::core::CompressionType;

TEST_CASE("Full encrypt-decrypt workflow", "[integration][workflow]") {
    CryptoEngine engine;
    engine.initialize();
    
    SECTION("AES-256-GCM with Argon2id") {
        std::string password = "MySecurePassword123!";
        std::string plaintext = "This is a secret message that needs encryption.";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        // Encryption
        EncryptionConfig config;
        config.algorithm = AlgorithmType::AES_256_GCM;
        config.kdf = KDFType::ARGON2ID;
        config.kdf_iterations = 3;  // Fast for testing
        config.kdf_memory_kb = 65536;
        
        auto salt = engine.generate_salt(32);
        auto nonce = engine.generate_nonce(12);
        config.nonce = nonce;
        
        auto key = engine.derive_key(password, salt, config);
        auto algorithm = engine.get_algorithm(config.algorithm);
        
        auto encrypted = algorithm->encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        // Decryption (need auth tag for GCM)
        if (encrypted.tag.has_value()) {
            config.tag = encrypted.tag.value();
        }
        auto key2 = engine.derive_key(password, salt, config);
        auto decrypted = algorithm->decrypt(encrypted.data, key2, config);
        
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == pt);
        
        std::string result(decrypted.data.begin(), decrypted.data.end());
        REQUIRE(result == plaintext);
    }
    
    SECTION("Wrong password fails") {
        std::string password = "CorrectPassword";
        std::string wrong_password = "WrongPassword";
        std::string plaintext = "Secret data";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        EncryptionConfig config;
        config.algorithm = AlgorithmType::AES_256_GCM;
        config.kdf = KDFType::ARGON2ID;
        config.kdf_iterations = 3;  // Fast for testing
        config.kdf_memory_kb = 65536;
        
        auto salt = engine.generate_salt(32);
        auto nonce = engine.generate_nonce(12);
        config.nonce = nonce;
        
        auto key = engine.derive_key(password, salt, config);
        auto algorithm = engine.get_algorithm(config.algorithm);
        auto encrypted = algorithm->encrypt(pt, key, config);
        
        // Try to decrypt with wrong password (need tag)
        if (encrypted.tag.has_value()) {
            config.tag = encrypted.tag.value();
        }
        auto wrong_key = engine.derive_key(wrong_password, salt, config);
        auto decrypted = algorithm->decrypt(encrypted.data, wrong_key, config);
        
        // Should fail (auth tag mismatch)
        REQUIRE_FALSE(decrypted.success);
    }
}

TEST_CASE("Compression + Encryption workflow", "[integration][compression]") {
    CryptoEngine engine;
    engine.initialize();
    
    SECTION("Compress then encrypt") {
        std::string plaintext(1000, 'A');  // Highly compressible
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        // Step 1: Compress (use ZLIB - more reliable than LZMA)
        auto compressor = CompressionService::create(CompressionType::ZLIB);
        auto compressed = compressor->compress(pt, 9);
        REQUIRE(compressed.success);
        REQUIRE(compressed.data.size() < pt.size());
        
        // Step 2: Encrypt
        std::string password = "TestPassword";
        EncryptionConfig config;
        config.algorithm = AlgorithmType::AES_256_GCM;
        config.kdf = KDFType::ARGON2ID;  // Faster than PBKDF2
        config.kdf_iterations = 3;
        config.kdf_memory_kb = 65536;
        
        auto salt = engine.generate_salt(32);
        auto nonce = engine.generate_nonce(12);
        config.nonce = nonce;
        
        auto key = engine.derive_key(password, salt, config);
        auto algorithm = engine.get_algorithm(config.algorithm);
        
        auto encrypted = algorithm->encrypt(compressed.data, key, config);
        REQUIRE(encrypted.success);
        
        // Step 3: Decrypt (need to set auth tag for GCM)
        if (encrypted.tag.has_value()) {
            config.tag = encrypted.tag.value();
        }
        auto key2 = engine.derive_key(password, salt, config);
        auto decrypted = algorithm->decrypt(encrypted.data, key2, config);
        REQUIRE(decrypted.success);
        
        // Step 4: Decompress
        auto decompressor = CompressionService::create(CompressionType::ZLIB);
        auto decompressed = decompressor->decompress(decrypted.data);
        REQUIRE(decompressed.success);
        REQUIRE(decompressed.data == pt);
    }
}

TEST_CASE("Multiple files workflow", "[integration][multiple]") {
    CryptoEngine engine;
    engine.initialize();
    
    std::string password = "SharedPassword";
    
    SECTION("Same password, different salts") {
        std::vector<std::string> plaintexts = {
            "File 1 contents",
            "File 2 different data",
            "File 3 more information"
        };
        
        EncryptionConfig config;
        config.algorithm = AlgorithmType::AES_256_GCM;
        config.kdf = KDFType::ARGON2ID;
        config.kdf_iterations = 3;  // Fast for testing
        config.kdf_memory_kb = 65536;
        
        std::vector<std::vector<uint8_t>> ciphertexts;
        std::vector<std::vector<uint8_t>> salts;
        std::vector<std::vector<uint8_t>> nonces;
        std::vector<std::vector<uint8_t>> tags;  // Store auth tags
        
        // Encrypt all files
        for (const auto& pt_str : plaintexts) {
            std::vector<uint8_t> pt(pt_str.begin(), pt_str.end());
            
            auto salt = engine.generate_salt(32);
            auto nonce = engine.generate_nonce(12);
            config.nonce = nonce;
            
            auto key = engine.derive_key(password, salt, config);
            auto algorithm = engine.get_algorithm(config.algorithm);
            
            auto encrypted = algorithm->encrypt(pt, key, config);
            REQUIRE(encrypted.success);
            
            ciphertexts.push_back(encrypted.data);
            salts.push_back(salt);
            nonces.push_back(nonce);
            if (encrypted.tag.has_value()) {
                tags.push_back(encrypted.tag.value());
            }
        }
        
        // All ciphertexts should be different (different nonces)
        REQUIRE(ciphertexts[0] != ciphertexts[1]);
        REQUIRE(ciphertexts[1] != ciphertexts[2]);
        
        // Decrypt all files
        for (size_t i = 0; i < plaintexts.size(); ++i) {
            config.nonce = nonces[i];
            if (i < tags.size()) {
                config.tag = tags[i];  // Set auth tag
            }
            auto key = engine.derive_key(password, salts[i], config);
            auto algorithm = engine.get_algorithm(config.algorithm);
            
            auto decrypted = algorithm->decrypt(ciphertexts[i], key, config);
            REQUIRE(decrypted.success);
            
            std::string result(decrypted.data.begin(), decrypted.data.end());
            REQUIRE(result == plaintexts[i]);
        }
    }
}
