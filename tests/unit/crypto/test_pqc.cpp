/**
 * @file test_pqc.cpp
 * @brief Unit tests for Post-Quantum Cryptography (Kyber, Dilithium)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "filevault/algorithms/pqc/post_quantum.hpp"
#include <vector>
#include <random>

using namespace filevault::algorithms::pqc;
using namespace filevault::core;

// ============================================================================
// Kyber Key Encapsulation Tests
// ============================================================================

TEST_CASE("Kyber Key Generation", "[pqc][kyber]") {
    auto variant = GENERATE(
        Kyber::Variant::Kyber512,
        Kyber::Variant::Kyber768,
        Kyber::Variant::Kyber1024
    );
    
    Kyber kyber(variant);
    auto keypair = kyber.generate_keypair();
    
    REQUIRE_FALSE(keypair.public_key.empty());
    REQUIRE_FALSE(keypair.private_key.empty());
    REQUIRE_FALSE(keypair.algorithm.empty());
    
    INFO("Public key size: " << keypair.public_key.size());
    INFO("Private key size: " << keypair.private_key.size());
}

TEST_CASE("Kyber Encapsulation", "[pqc][kyber]") {
    auto variant = GENERATE(
        Kyber::Variant::Kyber512,
        Kyber::Variant::Kyber768,
        Kyber::Variant::Kyber1024
    );
    
    Kyber kyber(variant);
    auto keypair = kyber.generate_keypair();
    
    EncryptionConfig config;
    auto result = kyber.encrypt({}, keypair.public_key, config);
    
    REQUIRE(result.success);
    REQUIRE_FALSE(result.data.empty());  // Encapsulated key
    REQUIRE(result.nonce.has_value());  // Shared secret
    REQUIRE(result.nonce->size() == 32);  // 256-bit shared secret
}

TEST_CASE("Kyber Decapsulation", "[pqc][kyber]") {
    auto variant = GENERATE(
        Kyber::Variant::Kyber512,
        Kyber::Variant::Kyber768,
        Kyber::Variant::Kyber1024
    );
    
    Kyber kyber(variant);
    auto keypair = kyber.generate_keypair();
    
    EncryptionConfig config;
    
    // Encapsulate
    auto enc_result = kyber.encrypt({}, keypair.public_key, config);
    REQUIRE(enc_result.success);
    
    // Decapsulate
    auto dec_result = kyber.decrypt(enc_result.data, keypair.private_key, config);
    REQUIRE(dec_result.success);
    
    // Shared secrets should match
    REQUIRE(enc_result.nonce.has_value());
    REQUIRE(dec_result.data == *enc_result.nonce);
}

TEST_CASE("Kyber Decapsulation with Wrong Key", "[pqc][kyber]") {
    Kyber kyber(Kyber::Variant::Kyber768);
    
    auto keypair1 = kyber.generate_keypair();
    auto keypair2 = kyber.generate_keypair();  // Different keypair
    
    EncryptionConfig config;
    
    // Encapsulate with keypair1's public key
    auto enc_result = kyber.encrypt({}, keypair1.public_key, config);
    REQUIRE(enc_result.success);
    
    // Try to decapsulate with keypair2's private key
    auto dec_result = kyber.decrypt(enc_result.data, keypair2.private_key, config);
    
    // Should succeed but produce different shared secret
    if (dec_result.success) {
        REQUIRE(dec_result.data != *enc_result.nonce);
    }
}

TEST_CASE("Kyber Security Level", "[pqc][kyber]") {
    SECTION("Kyber512") {
        Kyber kyber(Kyber::Variant::Kyber512);
        CHECK(kyber.is_suitable_for(SecurityLevel::MEDIUM));
    }
    
    SECTION("Kyber768") {
        Kyber kyber(Kyber::Variant::Kyber768);
        CHECK(kyber.is_suitable_for(SecurityLevel::STRONG));
    }
    
    SECTION("Kyber1024") {
        Kyber kyber(Kyber::Variant::Kyber1024);
        CHECK(kyber.is_suitable_for(SecurityLevel::PARANOID));
    }
}

// ============================================================================
// Dilithium Digital Signature Tests
// ============================================================================

TEST_CASE("Dilithium Key Generation", "[pqc][dilithium]") {
    auto variant = GENERATE(
        Dilithium::Variant::Dilithium2,
        Dilithium::Variant::Dilithium3,
        Dilithium::Variant::Dilithium5
    );
    
    Dilithium dilithium(variant);
    auto keypair = dilithium.generate_keypair();
    
    REQUIRE_FALSE(keypair.public_key.empty());
    REQUIRE_FALSE(keypair.private_key.empty());
    REQUIRE_FALSE(keypair.algorithm.empty());
}

TEST_CASE("Dilithium Sign and Verify", "[pqc][dilithium]") {
    auto variant = GENERATE(
        Dilithium::Variant::Dilithium2,
        Dilithium::Variant::Dilithium3,
        Dilithium::Variant::Dilithium5
    );
    
    Dilithium dilithium(variant);
    auto keypair = dilithium.generate_keypair();
    
    std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    
    auto signature = dilithium.sign(message, keypair.private_key);
    REQUIRE_FALSE(signature.empty());
    
    bool valid = dilithium.verify(message, signature, keypair.public_key);
    REQUIRE(valid);
}

TEST_CASE("Dilithium Verify Modified Message", "[pqc][dilithium]") {
    Dilithium dilithium(Dilithium::Variant::Dilithium3);
    auto keypair = dilithium.generate_keypair();
    
    std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o'};
    auto signature = dilithium.sign(message, keypair.private_key);
    
    // Modify message
    std::vector<uint8_t> modified = {'H', 'e', 'l', 'l', '0'};  // Changed 'o' to '0'
    
    bool valid = dilithium.verify(modified, signature, keypair.public_key);
    REQUIRE_FALSE(valid);
}

TEST_CASE("Dilithium Verify Modified Signature", "[pqc][dilithium]") {
    Dilithium dilithium(Dilithium::Variant::Dilithium3);
    auto keypair = dilithium.generate_keypair();
    
    std::vector<uint8_t> message = {'T', 'e', 's', 't'};
    auto signature = dilithium.sign(message, keypair.private_key);
    
    // Flip a bit in signature
    if (!signature.empty()) {
        signature[0] ^= 0x01;
    }
    
    bool valid = dilithium.verify(message, signature, keypair.public_key);
    REQUIRE_FALSE(valid);
}

TEST_CASE("Dilithium Verify with Wrong Public Key", "[pqc][dilithium]") {
    Dilithium dilithium(Dilithium::Variant::Dilithium3);
    auto keypair1 = dilithium.generate_keypair();
    auto keypair2 = dilithium.generate_keypair();
    
    std::vector<uint8_t> message = {'D', 'a', 't', 'a'};
    auto signature = dilithium.sign(message, keypair1.private_key);
    
    // Try to verify with different public key
    bool valid = dilithium.verify(message, signature, keypair2.public_key);
    REQUIRE_FALSE(valid);
}

TEST_CASE("Dilithium Sign Empty Message", "[pqc][dilithium]") {
    Dilithium dilithium(Dilithium::Variant::Dilithium2);
    auto keypair = dilithium.generate_keypair();
    
    std::vector<uint8_t> empty_message;
    
    auto signature = dilithium.sign(empty_message, keypair.private_key);
    REQUIRE_FALSE(signature.empty());
    
    bool valid = dilithium.verify(empty_message, signature, keypair.public_key);
    REQUIRE(valid);
}

TEST_CASE("Dilithium Sign Large Message", "[pqc][dilithium]") {
    Dilithium dilithium(Dilithium::Variant::Dilithium3);
    auto keypair = dilithium.generate_keypair();
    
    // 64KB message
    std::vector<uint8_t> large_message(64 * 1024);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (auto& byte : large_message) {
        byte = static_cast<uint8_t>(dis(gen));
    }
    
    auto signature = dilithium.sign(large_message, keypair.private_key);
    REQUIRE_FALSE(signature.empty());
    
    bool valid = dilithium.verify(large_message, signature, keypair.public_key);
    REQUIRE(valid);
}

// ============================================================================
// KyberHybrid (Kyber + AES-256-GCM) Tests
// ============================================================================

TEST_CASE("KyberHybrid Encrypt/Decrypt", "[pqc][hybrid]") {
    auto variant = GENERATE(
        Kyber::Variant::Kyber512,
        Kyber::Variant::Kyber768,
        Kyber::Variant::Kyber1024
    );
    
    KyberHybrid hybrid(variant);
    auto keypair = hybrid.generate_keypair();
    
    std::vector<uint8_t> plaintext = {
        'S', 'e', 'c', 'r', 'e', 't', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'
    };
    
    EncryptionConfig config;
    
    auto enc_result = hybrid.encrypt(plaintext, keypair.public_key, config);
    REQUIRE(enc_result.success);
    REQUIRE_FALSE(enc_result.data.empty());
    REQUIRE(enc_result.data.size() > plaintext.size());  // Should be larger due to KEM overhead
    
    auto dec_result = hybrid.decrypt(enc_result.data, keypair.private_key, config);
    REQUIRE(dec_result.success);
    REQUIRE(dec_result.data == plaintext);
}

TEST_CASE("KyberHybrid Encrypt/Decrypt Empty Data", "[pqc][hybrid]") {
    KyberHybrid hybrid(Kyber::Variant::Kyber768);
    auto keypair = hybrid.generate_keypair();
    
    std::vector<uint8_t> plaintext;
    
    EncryptionConfig config;
    
    auto enc_result = hybrid.encrypt(plaintext, keypair.public_key, config);
    REQUIRE(enc_result.success);
    
    auto dec_result = hybrid.decrypt(enc_result.data, keypair.private_key, config);
    REQUIRE(dec_result.success);
    REQUIRE(dec_result.data == plaintext);
}

TEST_CASE("KyberHybrid Encrypt/Decrypt Large Data", "[pqc][hybrid]") {
    KyberHybrid hybrid(Kyber::Variant::Kyber768);
    auto keypair = hybrid.generate_keypair();
    
    // 64KB of random data
    std::vector<uint8_t> plaintext(64 * 1024);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (auto& byte : plaintext) {
        byte = static_cast<uint8_t>(dis(gen));
    }
    
    EncryptionConfig config;
    
    auto enc_result = hybrid.encrypt(plaintext, keypair.public_key, config);
    REQUIRE(enc_result.success);
    
    auto dec_result = hybrid.decrypt(enc_result.data, keypair.private_key, config);
    REQUIRE(dec_result.success);
    REQUIRE(dec_result.data == plaintext);
}

TEST_CASE("KyberHybrid Decrypt with Wrong Key", "[pqc][hybrid]") {
    KyberHybrid hybrid(Kyber::Variant::Kyber768);
    
    auto keypair1 = hybrid.generate_keypair();
    auto keypair2 = hybrid.generate_keypair();
    
    std::vector<uint8_t> plaintext = {'T', 'e', 's', 't'};
    
    EncryptionConfig config;
    
    auto enc_result = hybrid.encrypt(plaintext, keypair1.public_key, config);
    REQUIRE(enc_result.success);
    
    // Try to decrypt with wrong key - should fail
    auto dec_result = hybrid.decrypt(enc_result.data, keypair2.private_key, config);
    
    // Either fails or produces wrong plaintext
    if (dec_result.success) {
        REQUIRE(dec_result.data != plaintext);
    }
}

TEST_CASE("KyberHybrid Tampered Ciphertext", "[pqc][hybrid]") {
    KyberHybrid hybrid(Kyber::Variant::Kyber768);
    auto keypair = hybrid.generate_keypair();
    
    std::vector<uint8_t> plaintext = {'S', 'e', 'c', 'r', 'e', 't'};
    
    EncryptionConfig config;
    
    auto enc_result = hybrid.encrypt(plaintext, keypair.public_key, config);
    REQUIRE(enc_result.success);
    
    // Tamper with ciphertext
    if (enc_result.data.size() > 10) {
        enc_result.data[enc_result.data.size() - 5] ^= 0xFF;
    }
    
    // Decryption should fail (integrity check)
    auto dec_result = hybrid.decrypt(enc_result.data, keypair.private_key, config);
    REQUIRE_FALSE(dec_result.success);
}

TEST_CASE("KyberHybrid Name", "[pqc][hybrid]") {
    KyberHybrid hybrid(Kyber::Variant::Kyber768);
    REQUIRE(hybrid.name().find("Hybrid") != std::string::npos);
}

// ============================================================================
// Algorithm Type Tests
// ============================================================================

TEST_CASE("Kyber Algorithm Types", "[pqc][types]") {
    SECTION("Kyber512") {
        Kyber kyber(Kyber::Variant::Kyber512);
        REQUIRE(kyber.type() == AlgorithmType::KYBER_512);
        REQUIRE(kyber.name() == "Kyber-512");
    }
    
    SECTION("Kyber768") {
        Kyber kyber(Kyber::Variant::Kyber768);
        REQUIRE(kyber.type() == AlgorithmType::KYBER_768);
        REQUIRE(kyber.name() == "Kyber-768");
    }
    
    SECTION("Kyber1024") {
        Kyber kyber(Kyber::Variant::Kyber1024);
        REQUIRE(kyber.type() == AlgorithmType::KYBER_1024);
        REQUIRE(kyber.name() == "Kyber-1024");
    }
}

TEST_CASE("Dilithium Names", "[pqc][types]") {
    SECTION("Dilithium2") {
        Dilithium dil(Dilithium::Variant::Dilithium2);
        REQUIRE(dil.name() == "Dilithium2");
    }
    
    SECTION("Dilithium3") {
        Dilithium dil(Dilithium::Variant::Dilithium3);
        REQUIRE(dil.name() == "Dilithium3");
    }
    
    SECTION("Dilithium5") {
        Dilithium dil(Dilithium::Variant::Dilithium5);
        REQUIRE(dil.name() == "Dilithium5");
    }
}
