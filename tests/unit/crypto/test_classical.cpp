#include <catch2/catch_test_macros.hpp>
#include "filevault/algorithms/classical/caesar.hpp"
#include "filevault/algorithms/classical/vigenere.hpp"
#include "filevault/algorithms/classical/playfair.hpp"
#include "filevault/algorithms/classical/hill.hpp"
#include "filevault/algorithms/classical/substitution.hpp"
#include <vector>
#include <string>

using namespace filevault::algorithms::classical;
using namespace filevault::core;

TEST_CASE("Caesar cipher encryption/decryption", "[classical][caesar]") {
    Caesar cipher;
    EncryptionConfig config;
    
    SECTION("Basic encryption with shift 3") {
        std::string plaintext = "HELLO";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key = {3};  // Shift by 3
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        std::string ciphertext(encrypted.data.begin(), encrypted.data.end());
        REQUIRE(ciphertext == "KHOOR");
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == pt);
    }
    
    SECTION("Wrap around alphabet") {
        std::string plaintext = "XYZ";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key = {3};
        
        auto encrypted = cipher.encrypt(pt, key, config);
        std::string ciphertext(encrypted.data.begin(), encrypted.data.end());
        REQUIRE(ciphertext == "ABC");
    }
    
    SECTION("Preserve non-alphabetic characters") {
        std::string plaintext = "Hello, World!";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key = {13};  // ROT13
        
        auto encrypted = cipher.encrypt(pt, key, config);
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.data == pt);
    }
}

TEST_CASE("Vigenère cipher encryption/decryption", "[classical][vigenere]") {
    Vigenere cipher;
    EncryptionConfig config;
    
    SECTION("Basic encryption with keyword") {
        std::string plaintext = "ATTACKATDAWN";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        std::string keyword = "LEMON";
        std::vector<uint8_t> key(keyword.begin(), keyword.end());
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        std::string ciphertext(encrypted.data.begin(), encrypted.data.end());
        REQUIRE(ciphertext == "LXFOPVEFRNHR");
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == pt);
    }
    
    SECTION("Case insensitive") {
        std::string plaintext = "Hello World";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::string keyword = "KEY";
        std::vector<uint8_t> key(keyword.begin(), keyword.end());
        
        auto encrypted = cipher.encrypt(pt, key, config);
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        
        // Should preserve original case and spaces
        std::string result(decrypted.data.begin(), decrypted.data.end());
        REQUIRE(result == plaintext);
    }
}

TEST_CASE("Playfair cipher encryption/decryption", "[classical][playfair]") {
    Playfair cipher;
    EncryptionConfig config;
    
    SECTION("Basic encryption") {
        std::string plaintext = "HELLO";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        std::string keyword = "KEYWORD";
        std::vector<uint8_t> key(keyword.begin(), keyword.end());
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        
        // Playfair cipher may add X for padding
        std::string result(decrypted.data.begin(), decrypted.data.end());
        REQUIRE(result.substr(0, 5) == "HELLO");
    }
}

TEST_CASE("Hill cipher encryption/decryption", "[classical][hill]") {
    HillCipher cipher;
    EncryptionConfig config;
    
    SECTION("2x2 matrix encryption") {
        std::string plaintext = "HELP";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        // Valid invertible matrix key
        std::vector<uint8_t> key = {3, 3, 2, 5};  // [[3,3],[2,5]]
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        
        std::string result(decrypted.data.begin(), decrypted.data.end());
        REQUIRE(result == "HELP");
    }
    
    SECTION("Odd length padding") {
        std::string plaintext = "HEL";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key = {3, 3, 2, 5};
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        // Should add 'X' padding
        REQUIRE(encrypted.data.size() == 4);  // HELX
    }
}

TEST_CASE("Substitution cipher encryption/decryption", "[classical][substitution]") {
    SubstitutionCipher cipher;
    EncryptionConfig config;
    
    SECTION("Reverse alphabet substitution") {
        std::string plaintext = "HELLO";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        
        // Generate key from password (KDF will create permutation)
        std::string password = "testkey";
        std::vector<uint8_t> key(password.begin(), password.end());
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == pt);
    }
    
    SECTION("Preserve case and punctuation") {
        std::string plaintext = "Hello, World!";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::string password = "key123";
        std::vector<uint8_t> key(password.begin(), password.end());
        
        auto encrypted = cipher.encrypt(pt, key, config);
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        
        std::string result(decrypted.data.begin(), decrypted.data.end());
        REQUIRE(result == plaintext);
    }
}
