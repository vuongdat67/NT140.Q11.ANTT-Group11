/**
 * @file test_non_aead_ciphers.cpp
 * @brief Tests for non-AEAD symmetric ciphers (CBC, CTR, 3DES)
 * 
 * WARNING: These ciphers do NOT provide authentication.
 * For production use, always prefer AEAD ciphers (GCM, ChaCha20-Poly1305).
 */

#include <catch2/catch_all.hpp>
#include "filevault/core/crypto_engine.hpp"
#include "filevault/algorithms/symmetric/aes_cbc.hpp"
#include "filevault/algorithms/symmetric/aes_ctr.hpp"
#include "filevault/algorithms/symmetric/triple_des.hpp"
#include <botan/auto_rng.h>
#include <string>
#include <vector>

using namespace filevault::core;
using namespace filevault::algorithms::symmetric;

// ============================================================================
// AES-CBC Tests
// ============================================================================

TEST_CASE("AES-128-CBC basic encrypt/decrypt", "[aes-cbc][symmetric]") {
    AES_CBC cipher(128);
    
    REQUIRE(cipher.name() == "AES-128-CBC");
    REQUIRE(cipher.key_size() == 16);
    REQUIRE(cipher.iv_size() == 16);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(16);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Hello, AES-CBC encryption test!";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_128_CBC;
    
    // Encrypt
    auto encrypted = cipher.encrypt(data, key, config);
    REQUIRE(encrypted.success);
    REQUIRE(!encrypted.data.empty());
    REQUIRE(encrypted.nonce.has_value()); // IV stored in nonce
    
    // Decrypt - need to pass IV from encrypt result
    EncryptionConfig decrypt_config;
    decrypt_config.algorithm = AlgorithmType::AES_128_CBC;
    decrypt_config.nonce = encrypted.nonce;
    
    auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
    REQUIRE(decrypted.success);
    REQUIRE(!decrypted.data.empty());
    
    std::string result(decrypted.data.begin(), decrypted.data.end());
    REQUIRE(result == plaintext);
}

TEST_CASE("AES-192-CBC basic encrypt/decrypt", "[aes-cbc][symmetric]") {
    AES_CBC cipher(192);
    
    REQUIRE(cipher.name() == "AES-192-CBC");
    REQUIRE(cipher.key_size() == 24);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(24);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Testing AES-192-CBC mode!";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_192_CBC;
    
    auto encrypted = cipher.encrypt(data, key, config);
    REQUIRE(encrypted.success);
    
    // Decrypt with IV from encrypt result
    EncryptionConfig decrypt_config;
    decrypt_config.algorithm = AlgorithmType::AES_192_CBC;
    decrypt_config.nonce = encrypted.nonce;
    
    auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
    REQUIRE(decrypted.success);
    
    std::string result(decrypted.data.begin(), decrypted.data.end());
    REQUIRE(result == plaintext);
}

TEST_CASE("AES-256-CBC basic encrypt/decrypt", "[aes-cbc][symmetric]") {
    AES_CBC cipher(256);
    
    REQUIRE(cipher.name() == "AES-256-CBC");
    REQUIRE(cipher.key_size() == 32);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(32);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "AES-256-CBC - the strongest AES-CBC variant!";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_256_CBC;
    
    auto encrypted = cipher.encrypt(data, key, config);
    REQUIRE(encrypted.success);
    
    // Decrypt with IV from encrypt result
    EncryptionConfig decrypt_config;
    decrypt_config.algorithm = AlgorithmType::AES_256_CBC;
    decrypt_config.nonce = encrypted.nonce;
    
    auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
    REQUIRE(decrypted.success);
    
    std::string result(decrypted.data.begin(), decrypted.data.end());
    REQUIRE(result == plaintext);
}

TEST_CASE("AES-CBC PKCS7 padding", "[aes-cbc][padding]") {
    AES_CBC cipher(128);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(16);
    rng.randomize(key.data(), key.size());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_128_CBC;
    
    // Test different plaintext sizes to verify padding
    for (size_t size = 1; size <= 32; ++size) {
        std::vector<uint8_t> data(size, 'A');
        
        auto encrypted = cipher.encrypt(data, key, config);
        REQUIRE(encrypted.success);
        
        // Decrypt with IV from encrypt result
        EncryptionConfig decrypt_config;
        decrypt_config.algorithm = AlgorithmType::AES_128_CBC;
        decrypt_config.nonce = encrypted.nonce;
        
        auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
        REQUIRE(decrypted.success);
        
        REQUIRE(decrypted.data == data);
    }
}

TEST_CASE("AES-CBC unique IV per encryption", "[aes-cbc][security]") {
    AES_CBC cipher(256);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(32);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Same plaintext, different IV";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_256_CBC;
    
    auto encrypted1 = cipher.encrypt(data, key, config);
    auto encrypted2 = cipher.encrypt(data, key, config);
    
    REQUIRE(encrypted1.success);
    REQUIRE(encrypted2.success);
    
    // Different encryptions should produce different ciphertexts (due to random IV)
    REQUIRE(encrypted1.data != encrypted2.data);
    
    // Both should decrypt to same plaintext
    EncryptionConfig decrypt_config1, decrypt_config2;
    decrypt_config1.algorithm = AlgorithmType::AES_256_CBC;
    decrypt_config1.nonce = encrypted1.nonce;
    decrypt_config2.algorithm = AlgorithmType::AES_256_CBC;
    decrypt_config2.nonce = encrypted2.nonce;
    
    auto decrypted1 = cipher.decrypt(encrypted1.data, key, decrypt_config1);
    auto decrypted2 = cipher.decrypt(encrypted2.data, key, decrypt_config2);
    
    REQUIRE(decrypted1.success);
    REQUIRE(decrypted2.success);
    REQUIRE(decrypted1.data == data);
    REQUIRE(decrypted2.data == data);
}

// ============================================================================
// AES-CTR Tests
// ============================================================================

TEST_CASE("AES-128-CTR basic encrypt/decrypt", "[aes-ctr][symmetric]") {
    AES_CTR cipher(128);
    
    REQUIRE(cipher.name() == "AES-128-CTR");
    REQUIRE(cipher.key_size() == 16);
    REQUIRE(cipher.nonce_size() == 16);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(16);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Hello, AES-CTR encryption test!";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_128_CTR;
    
    auto encrypted = cipher.encrypt(data, key, config);
    REQUIRE(encrypted.success);
    REQUIRE(!encrypted.data.empty());
    
    // Decrypt with nonce from encrypt result
    EncryptionConfig decrypt_config;
    decrypt_config.algorithm = AlgorithmType::AES_128_CTR;
    decrypt_config.nonce = encrypted.nonce;
    
    auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
    REQUIRE(decrypted.success);
    REQUIRE(!decrypted.data.empty());
    
    std::string result(decrypted.data.begin(), decrypted.data.end());
    REQUIRE(result == plaintext);
}

TEST_CASE("AES-256-CTR stream cipher property", "[aes-ctr][symmetric]") {
    AES_CTR cipher(256);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(32);
    rng.randomize(key.data(), key.size());
    
    // CTR mode is a stream cipher - ciphertext size equals plaintext size
    for (size_t size = 1; size <= 100; ++size) {
        std::vector<uint8_t> data(size, 'X');
        
        EncryptionConfig config;
        config.algorithm = AlgorithmType::AES_256_CTR;
        
        auto encrypted = cipher.encrypt(data, key, config);
        REQUIRE(encrypted.success);
        
        // Ciphertext should equal plaintext size (no padding in CTR mode)
        REQUIRE(encrypted.data.size() == size);
        
        // Decrypt with nonce from encrypt result
        EncryptionConfig decrypt_config;
        decrypt_config.algorithm = AlgorithmType::AES_256_CTR;
        decrypt_config.nonce = encrypted.nonce;
        
        auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == data);
    }
}

TEST_CASE("AES-CTR unique nonce per encryption", "[aes-ctr][security]") {
    AES_CTR cipher(256);
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(32);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Same plaintext";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::AES_256_CTR;
    
    auto encrypted1 = cipher.encrypt(data, key, config);
    auto encrypted2 = cipher.encrypt(data, key, config);
    
    REQUIRE(encrypted1.success);
    REQUIRE(encrypted2.success);
    
    // Different nonces should produce different ciphertexts
    REQUIRE(encrypted1.data != encrypted2.data);
    
    // Decrypt with nonce from encrypt result
    EncryptionConfig decrypt_config1, decrypt_config2;
    decrypt_config1.algorithm = AlgorithmType::AES_256_CTR;
    decrypt_config1.nonce = encrypted1.nonce;
    decrypt_config2.algorithm = AlgorithmType::AES_256_CTR;
    decrypt_config2.nonce = encrypted2.nonce;
    
    auto decrypted1 = cipher.decrypt(encrypted1.data, key, decrypt_config1);
    auto decrypted2 = cipher.decrypt(encrypted2.data, key, decrypt_config2);
    
    REQUIRE(decrypted1.success);
    REQUIRE(decrypted2.success);
    REQUIRE(decrypted1.data == data);
    REQUIRE(decrypted2.data == data);
}

// ============================================================================
// Triple-DES Tests
// ============================================================================

TEST_CASE("3DES-CBC basic encrypt/decrypt", "[3des][symmetric][legacy]") {
    TripleDES cipher;
    
    REQUIRE(cipher.name() == "3DES-CBC");
    REQUIRE(cipher.key_size() == 24);  // 168-bit effective, 192-bit with parity
    REQUIRE(cipher.iv_size() == 8);     // 64-bit block size
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(24);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Hello, Triple-DES test!";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::TRIPLE_DES_CBC;
    
    auto encrypted = cipher.encrypt(data, key, config);
    REQUIRE(encrypted.success);
    REQUIRE(!encrypted.data.empty());
    
    // Decrypt with IV from encrypt result
    EncryptionConfig decrypt_config;
    decrypt_config.algorithm = AlgorithmType::TRIPLE_DES_CBC;
    decrypt_config.nonce = encrypted.nonce;
    
    auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
    REQUIRE(decrypted.success);
    REQUIRE(!decrypted.data.empty());
    
    std::string result(decrypted.data.begin(), decrypted.data.end());
    REQUIRE(result == plaintext);
}

TEST_CASE("3DES-CBC various data sizes", "[3des][symmetric]") {
    TripleDES cipher;
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(24);
    rng.randomize(key.data(), key.size());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::TRIPLE_DES_CBC;
    
    for (size_t size = 1; size <= 32; ++size) {
        std::vector<uint8_t> data(size, 'B');
        
        auto encrypted = cipher.encrypt(data, key, config);
        REQUIRE(encrypted.success);
        
        // Decrypt with IV from encrypt result
        EncryptionConfig decrypt_config;
        decrypt_config.algorithm = AlgorithmType::TRIPLE_DES_CBC;
        decrypt_config.nonce = encrypted.nonce;
        
        auto decrypted = cipher.decrypt(encrypted.data, key, decrypt_config);
        REQUIRE(decrypted.success);
        
        REQUIRE(decrypted.data == data);
    }
}

TEST_CASE("3DES-CBC unique IV per encryption", "[3des][security]") {
    TripleDES cipher;
    
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> key(24);
    rng.randomize(key.data(), key.size());
    
    std::string plaintext = "Same data";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    EncryptionConfig config;
    config.algorithm = AlgorithmType::TRIPLE_DES_CBC;
    
    auto encrypted1 = cipher.encrypt(data, key, config);
    auto encrypted2 = cipher.encrypt(data, key, config);
    
    REQUIRE(encrypted1.success);
    REQUIRE(encrypted2.success);
    
    REQUIRE(encrypted1.data != encrypted2.data);
    
    // Decrypt with IV from encrypt result
    EncryptionConfig decrypt_config1, decrypt_config2;
    decrypt_config1.algorithm = AlgorithmType::TRIPLE_DES_CBC;
    decrypt_config1.nonce = encrypted1.nonce;
    decrypt_config2.algorithm = AlgorithmType::TRIPLE_DES_CBC;
    decrypt_config2.nonce = encrypted2.nonce;
    
    auto decrypted1 = cipher.decrypt(encrypted1.data, key, decrypt_config1);
    auto decrypted2 = cipher.decrypt(encrypted2.data, key, decrypt_config2);
    
    REQUIRE(decrypted1.success);
    REQUIRE(decrypted2.success);
    REQUIRE(decrypted1.data == data);
    REQUIRE(decrypted2.data == data);
}

// ============================================================================
// CryptoEngine Integration Tests
// ============================================================================

TEST_CASE("CryptoEngine registers non-AEAD ciphers", "[crypto-engine][integration]") {
    CryptoEngine engine;
    engine.initialize();
    
    // Check AES-CBC variants
    REQUIRE(engine.get_algorithm(AlgorithmType::AES_128_CBC) != nullptr);
    REQUIRE(engine.get_algorithm(AlgorithmType::AES_192_CBC) != nullptr);
    REQUIRE(engine.get_algorithm(AlgorithmType::AES_256_CBC) != nullptr);
    
    // Check AES-CTR variants
    REQUIRE(engine.get_algorithm(AlgorithmType::AES_128_CTR) != nullptr);
    REQUIRE(engine.get_algorithm(AlgorithmType::AES_192_CTR) != nullptr);
    REQUIRE(engine.get_algorithm(AlgorithmType::AES_256_CTR) != nullptr);
    
    // Check 3DES
    REQUIRE(engine.get_algorithm(AlgorithmType::TRIPLE_DES_CBC) != nullptr);
}

TEST_CASE("CryptoEngine parse non-AEAD algorithms", "[crypto-engine][parsing]") {
    // AES-CBC
    REQUIRE(CryptoEngine::parse_algorithm("aes-128-cbc") == AlgorithmType::AES_128_CBC);
    REQUIRE(CryptoEngine::parse_algorithm("aes-192-cbc") == AlgorithmType::AES_192_CBC);
    REQUIRE(CryptoEngine::parse_algorithm("aes-256-cbc") == AlgorithmType::AES_256_CBC);
    
    // AES-CTR
    REQUIRE(CryptoEngine::parse_algorithm("aes-128-ctr") == AlgorithmType::AES_128_CTR);
    REQUIRE(CryptoEngine::parse_algorithm("aes-192-ctr") == AlgorithmType::AES_192_CTR);
    REQUIRE(CryptoEngine::parse_algorithm("aes-256-ctr") == AlgorithmType::AES_256_CTR);
    
    // 3DES
    REQUIRE(CryptoEngine::parse_algorithm("3des") == AlgorithmType::TRIPLE_DES_CBC);
    REQUIRE(CryptoEngine::parse_algorithm("tripledes") == AlgorithmType::TRIPLE_DES_CBC);
    REQUIRE(CryptoEngine::parse_algorithm("triple-des") == AlgorithmType::TRIPLE_DES_CBC);
}

TEST_CASE("CryptoEngine algorithm names", "[crypto-engine][naming]") {
    REQUIRE(CryptoEngine::algorithm_name(AlgorithmType::AES_128_CBC) == "AES-128-CBC");
    REQUIRE(CryptoEngine::algorithm_name(AlgorithmType::AES_256_CTR) == "AES-256-CTR");
    REQUIRE(CryptoEngine::algorithm_name(AlgorithmType::TRIPLE_DES_CBC) == "3DES-CBC");
}

// ============================================================================
// Security Warning Tests
// ============================================================================

TEST_CASE("Non-AEAD ciphers warning", "[security][warning]") {
    // This test just documents the security concerns
    INFO("WARNING: AES-CBC and AES-CTR do NOT provide authentication!");
    INFO("They are vulnerable to padding oracle and bit-flipping attacks.");
    INFO("For production use, always prefer AEAD ciphers (AES-GCM, ChaCha20-Poly1305).");
    INFO("These modes are included for compatibility with legacy systems only.");
    
    REQUIRE(true); // Always pass, just for documentation
}
