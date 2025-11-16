#pragma once

#include "filevault/core/crypto_algorithm.hpp"
#include <string>

namespace filevault {
namespace algorithms {
namespace classical {

/**
 * @brief Caesar Cipher - Educational only
 * 
 * Simple substitution cipher that shifts characters by a fixed offset.
 * Used by Julius Caesar for military messages (~50 BC).
 * 
 * Security: COMPLETELY BROKEN - Only 26 possible keys!
 * Purpose: Educational demonstration of cryptanalysis
 */
class Caesar : public core::ICryptoAlgorithm {
public:
    explicit Caesar(int shift = 3);
    
    std::string name() const override { return "Caesar"; }
    core::AlgorithmType type() const override { return core::AlgorithmType::CAESAR; }
    size_t key_size() const override { return 4; } // Min for Argon2, use first byte as shift
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    bool is_suitable_for(core::SecurityLevel level) const override {
        (void)level; // Educational only - not for real security
        return false;
    }
    
    // Educational methods
    static std::string brute_force(const std::string& ciphertext);
    static double frequency_analysis(const std::string& text);
    
private:
    int shift_;
    
    char shift_char(char ch, int shift) const;
};

/**
 * @brief Vigenère Cipher - Polyalphabetic substitution
 * 
 * Uses a keyword to create multiple Caesar shifts.
 * Considered "le chiffre indéchiffrable" (unbreakable cipher) until 1863.
 * 
 * Security: BROKEN - Vulnerable to Kasiski examination
 * Purpose: Educational - shows improvement over monoalphabetic ciphers
 */
class Vigenere : public core::ICryptoAlgorithm {
public:
    explicit Vigenere(const std::string& keyword = "KEY");
    
    std::string name() const override { return "Vigenere"; }
    core::AlgorithmType type() const override { return core::AlgorithmType::VIGENERE; }
    size_t key_size() const override { return 32; } // Standard key size
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    bool is_suitable_for(core::SecurityLevel level) const override {
        (void)level; // Educational only - not for real security
        return false;
    }
    
    // Educational methods
    static std::vector<size_t> kasiski_examination(const std::string& ciphertext);
    static size_t estimate_key_length(const std::string& ciphertext);
    
private:
    std::string keyword_;
};

/**
 * @brief Playfair Cipher - Digraph substitution
 * 
 * Uses a 5x5 matrix based on a keyword.
 * First practical digraph substitution cipher.
 * 
 * Security: BROKEN - 600 possible digraphs still analyzable
 * Purpose: Educational - shows digraph cryptanalysis
 */
class Playfair : public core::ICryptoAlgorithm {
public:
    explicit Playfair(const std::string& keyword = "KEYWORD");
    
    std::string name() const override { return "Playfair"; }
    core::AlgorithmType type() const override { return core::AlgorithmType::PLAYFAIR; }
    size_t key_size() const override { return 32; } // Standard key size
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    bool is_suitable_for(core::SecurityLevel level) const override {
        (void)level; // Educational only - not for real security
        return false;
    }
    
private:
    char matrix_[5][5];
    void build_matrix(const std::string& keyword);
    std::pair<int, int> find_position(char ch) const;
};

} // namespace classical
} // namespace algorithms
} // namespace filevault
