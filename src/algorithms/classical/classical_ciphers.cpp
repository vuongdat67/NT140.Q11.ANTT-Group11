#include "filevault/algorithms/classical/classical_ciphers.hpp"
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace filevault {
namespace algorithms {
namespace classical {

// ============================================================================
// Caesar Cipher Implementation
// ============================================================================

Caesar::Caesar(int shift) : shift_(shift % 26) {}

char Caesar::shift_char(char ch, int shift) const {
    if (std::isalpha(ch)) {
        char base = std::isupper(ch) ? 'A' : 'a';
        return base + (ch - base + shift + 26) % 26;
    }
    return ch;
}

core::CryptoResult Caesar::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config)
{
    (void)config; // Unused for Caesar
    auto start = std::chrono::high_resolution_clock::now();
    
    int shift = shift_;
    if (!key.empty()) {
        shift = static_cast<int>(key[0]) % 26;
    }
    
    std::vector<uint8_t> result;
    result.reserve(plaintext.size());
    
    for (uint8_t byte : plaintext) {
        result.push_back(static_cast<uint8_t>(shift_char(static_cast<char>(byte), shift)));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    core::CryptoResult crypto_result;
    crypto_result.success = true;
    crypto_result.data = std::move(result);
    crypto_result.algorithm_used = core::AlgorithmType::CAESAR;
    crypto_result.original_size = plaintext.size();
    crypto_result.final_size = result.size();
    crypto_result.processing_time_ms = duration.count() / 1000.0;
    
    return crypto_result;
}

core::CryptoResult Caesar::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config)
{
    (void)config; // Unused for Caesar
    auto start = std::chrono::high_resolution_clock::now();
    
    int shift = shift_;
    if (!key.empty()) {
        shift = static_cast<int>(key[0]) % 26;
    }
    
    std::vector<uint8_t> result;
    result.reserve(ciphertext.size());
    
    for (uint8_t byte : ciphertext) {
        result.push_back(static_cast<uint8_t>(shift_char(static_cast<char>(byte), -shift)));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    core::CryptoResult crypto_result;
    crypto_result.success = true;
    crypto_result.data = std::move(result);
    crypto_result.algorithm_used = core::AlgorithmType::CAESAR;
    crypto_result.original_size = ciphertext.size();
    crypto_result.final_size = result.size();
    crypto_result.processing_time_ms = duration.count() / 1000.0;
    
    return crypto_result;
}

std::string Caesar::brute_force(const std::string& ciphertext) {
    std::ostringstream result;
    result << "Caesar Brute Force Attack:\n";
    result << "==========================\n\n";
    
    for (int shift = 0; shift < 26; ++shift) {
        result << "Shift " << std::setw(2) << shift << ": ";
        Caesar cipher(shift);
        
        std::vector<uint8_t> input(ciphertext.begin(), ciphertext.end());
        std::vector<uint8_t> dummy_key = {0};
        core::EncryptionConfig dummy_config;
        
        auto decrypted = cipher.decrypt(input, dummy_key, dummy_config);
        
        if (decrypted.success) {
            std::string text(decrypted.data.begin(), decrypted.data.end());
            result << text << "\n";
        }
    }
    
    return result.str();
}

// ============================================================================
// Vigenère Cipher Implementation
// ============================================================================

Vigenere::Vigenere(const std::string& keyword) : keyword_(keyword) {
    // Convert to uppercase
    std::transform(keyword_.begin(), keyword_.end(), keyword_.begin(), ::toupper);
}

core::CryptoResult Vigenere::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config)
{
    (void)config;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string keyword = keyword_;
    if (!key.empty()) {
        keyword = std::string(key.begin(), key.end());
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::toupper);
    }
    
    std::vector<uint8_t> result;
    result.reserve(plaintext.size());
    
    size_t key_index = 0;
    for (uint8_t byte : plaintext) {
        char ch = static_cast<char>(byte);
        
        if (std::isalpha(ch)) {
            char base = std::isupper(ch) ? 'A' : 'a';
            int shift = keyword[key_index % keyword.size()] - 'A';
            ch = base + (ch - base + shift) % 26;
            key_index++;
        }
        
        result.push_back(static_cast<uint8_t>(ch));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    core::CryptoResult crypto_result;
    crypto_result.success = true;
    crypto_result.data = std::move(result);
    crypto_result.algorithm_used = core::AlgorithmType::VIGENERE;
    crypto_result.original_size = plaintext.size();
    crypto_result.final_size = result.size();
    crypto_result.processing_time_ms = duration.count() / 1000.0;
    return crypto_result;
}

core::CryptoResult Vigenere::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config)
{
    (void)config;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string keyword = keyword_;
    if (!key.empty()) {
        keyword = std::string(key.begin(), key.end());
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::toupper);
    }
    
    std::vector<uint8_t> result;
    result.reserve(ciphertext.size());
    
    size_t key_index = 0;
    for (uint8_t byte : ciphertext) {
        char ch = static_cast<char>(byte);
        
        if (std::isalpha(ch)) {
            char base = std::isupper(ch) ? 'A' : 'a';
            int shift = keyword[key_index % keyword.size()] - 'A';
            ch = base + (ch - base - shift + 26) % 26;
            key_index++;
        }
        
        result.push_back(static_cast<uint8_t>(ch));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    core::CryptoResult crypto_result;
    crypto_result.success = true;
    crypto_result.data = std::move(result);
    crypto_result.algorithm_used = core::AlgorithmType::VIGENERE;
    crypto_result.original_size = ciphertext.size();
    crypto_result.final_size = result.size();
    crypto_result.processing_time_ms = duration.count() / 1000.0;
    return crypto_result;
}

// ============================================================================
// Playfair Cipher Implementation
// ============================================================================

Playfair::Playfair(const std::string& keyword) {
    build_matrix(keyword);
}

void Playfair::build_matrix(const std::string& keyword) {
    std::string key = keyword;
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    
    // Remove J (combine with I)
    key.erase(std::remove(key.begin(), key.end(), 'J'), key.end());
    
    // Add alphabet
    std::string alphabet = "ABCDEFGHIKLMNOPQRSTUVWXYZ"; // No J
    key += alphabet;
    
    // Remove duplicates
    std::string unique_key;
    std::unordered_map<char, bool> seen;
    
    for (char ch : key) {
        if (!seen[ch] && std::isalpha(ch)) {
            unique_key += ch;
            seen[ch] = true;
        }
    }
    
    // Fill matrix
    for (int i = 0; i < 25; ++i) {
        matrix_[i / 5][i % 5] = unique_key[i];
    }
}

std::pair<int, int> Playfair::find_position(char ch) const {
    ch = std::toupper(ch);
    if (ch == 'J') ch = 'I';
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (matrix_[i][j] == ch) {
                return {i, j};
            }
        }
    }
    return {0, 0};
}

core::CryptoResult Playfair::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config)
{
    (void)config;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (!key.empty()) {
        std::string keyword(key.begin(), key.end());
        build_matrix(keyword);
    }
    
    // Prepare plaintext: pairs of letters
    std::string text;
    for (uint8_t byte : plaintext) {
        char ch = std::toupper(static_cast<char>(byte));
        if (std::isalpha(ch)) {
            if (ch == 'J') ch = 'I';
            text += ch;
        }
    }
    
    // Add padding X if odd length
    if (text.length() % 2 != 0) {
        text += 'X';
    }
    
    std::vector<uint8_t> result;
    
    // Encrypt pairs
    for (size_t i = 0; i < text.length(); i += 2) {
        auto [r1, c1] = find_position(text[i]);
        auto [r2, c2] = find_position(text[i + 1]);
        
        char ch1, ch2;
        
        if (r1 == r2) {
            // Same row: shift right
            ch1 = matrix_[r1][(c1 + 1) % 5];
            ch2 = matrix_[r2][(c2 + 1) % 5];
        } else if (c1 == c2) {
            // Same column: shift down
            ch1 = matrix_[(r1 + 1) % 5][c1];
            ch2 = matrix_[(r2 + 1) % 5][c2];
        } else {
            // Rectangle: swap columns
            ch1 = matrix_[r1][c2];
            ch2 = matrix_[r2][c1];
        }
        
        result.push_back(static_cast<uint8_t>(ch1));
        result.push_back(static_cast<uint8_t>(ch2));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    core::CryptoResult crypto_result;
    crypto_result.success = true;
    crypto_result.data = std::move(result);
    crypto_result.algorithm_used = core::AlgorithmType::PLAYFAIR;
    crypto_result.original_size = text.length();
    crypto_result.final_size = result.size();
    crypto_result.processing_time_ms = duration.count() / 1000.0;
    return crypto_result;
}

core::CryptoResult Playfair::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config)
{
    (void)config;
    auto start = std::chrono::high_resolution_clock::now();
    
    if (!key.empty()) {
        std::string keyword(key.begin(), key.end());
        build_matrix(keyword);
    }
    
    std::string text(ciphertext.begin(), ciphertext.end());
    std::vector<uint8_t> result;
    
    // Decrypt pairs
    for (size_t i = 0; i < text.length(); i += 2) {
        auto [r1, c1] = find_position(text[i]);
        auto [r2, c2] = find_position(text[i + 1]);
        
        char ch1, ch2;
        
        if (r1 == r2) {
            // Same row: shift left
            ch1 = matrix_[r1][(c1 + 4) % 5];
            ch2 = matrix_[r2][(c2 + 4) % 5];
        } else if (c1 == c2) {
            // Same column: shift up
            ch1 = matrix_[(r1 + 4) % 5][c1];
            ch2 = matrix_[(r2 + 4) % 5][c2];
        } else {
            // Rectangle: swap columns
            ch1 = matrix_[r1][c2];
            ch2 = matrix_[r2][c1];
        }
        
        result.push_back(static_cast<uint8_t>(ch1));
        result.push_back(static_cast<uint8_t>(ch2));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    core::CryptoResult crypto_result;
    crypto_result.success = true;
    crypto_result.data = std::move(result);
    crypto_result.algorithm_used = core::AlgorithmType::PLAYFAIR;
    crypto_result.original_size = ciphertext.size();
    crypto_result.final_size = result.size();
    crypto_result.processing_time_ms = duration.count() / 1000.0;
    return crypto_result;
}

} // namespace classical
} // namespace algorithms
} // namespace filevault
