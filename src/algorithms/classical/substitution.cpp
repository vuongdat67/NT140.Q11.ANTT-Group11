#include "filevault/algorithms/classical/substitution.hpp"
#include <algorithm>
#include <set>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace classical {

SubstitutionCipher::SubstitutionMap SubstitutionCipher::parse_key(std::span<const uint8_t> key) {
    SubstitutionMap map;
    
    if (key.empty()) {
        // Default to identity mapping if key is empty
        for (size_t i = 0; i < 26; i++) {
            map[i] = 'A' + i;
        }
        return map;
    }
    
    // Generate substitution alphabet from binary key
    // Use key bytes to create a permutation of the alphabet
    std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    // Simple shuffle based on key bytes
    for (size_t i = 0; i < std::min(key.size(), size_t(26)); i++) {
        size_t j = key[i] % (26 - i);
        std::swap(alphabet[i], alphabet[i + j]);
    }
    
    for (size_t i = 0; i < 26; i++) {
        map[i] = alphabet[i];
    }
    
    return map;
}

bool SubstitutionCipher::is_valid_key(const SubstitutionMap& map) {
    // Check that all 26 letters are present (it's a valid permutation)
    std::set<char> letters;
    for (char c : map) {
        if (c < 'A' || c > 'Z') return false;
        letters.insert(c);
    }
    
    return letters.size() == 26;
}

SubstitutionCipher::SubstitutionMap SubstitutionCipher::create_reverse_map(const SubstitutionMap& forward_map) {
    SubstitutionMap reverse_map;
    
    for (size_t i = 0; i < 26; i++) {
        char cipher_letter = forward_map[i];
        int cipher_index = cipher_letter - 'A';
        if (cipher_index >= 0 && cipher_index < 26) {
            reverse_map[cipher_index] = 'A' + i;
        }
    }
    
    return reverse_map;
}

std::string SubstitutionCipher::apply_substitution(const std::string& text, const SubstitutionMap& map) {
    std::string result;
    result.reserve(text.length());
    
    for (char c : text) {
        if (isalpha(c)) {
            bool is_upper = isupper(c);
            char upper_c = toupper(c);
            int index = upper_c - 'A';
            
            if (index >= 0 && index < 26) {
                char substituted = map[index];
                result += is_upper ? substituted : tolower(substituted);
            } else {
                result += c;
            }
        } else {
            result += c;  // Keep non-alphabetic characters
        }
    }
    
    return result;
}

core::CryptoResult SubstitutionCipher::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    (void)config;
    core::CryptoResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto substitution_map = parse_key(key);
        
        if (!is_valid_key(substitution_map)) {
            result.success = false;
            result.error_message = "Invalid key: must be a valid 26-letter permutation";
            return result;
        }
        
        std::string text(plaintext.begin(), plaintext.end());
        std::string ciphertext = apply_substitution(text, substitution_map);
        
        result.success = true;
        result.data.assign(ciphertext.begin(), ciphertext.end());
        result.algorithm_used = type();
        result.original_size = plaintext.size();
        result.final_size = ciphertext.length();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

core::CryptoResult SubstitutionCipher::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    (void)config;
    core::CryptoResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto substitution_map = parse_key(key);
        
        if (!is_valid_key(substitution_map)) {
            result.success = false;
            result.error_message = "Invalid key: must be a valid 26-letter permutation";
            return result;
        }
        
        // Create reverse mapping for decryption
        auto reverse_map = create_reverse_map(substitution_map);
        
        std::string text(ciphertext.begin(), ciphertext.end());
        std::string plaintext = apply_substitution(text, reverse_map);
        
        result.success = true;
        result.data.assign(plaintext.begin(), plaintext.end());
        result.algorithm_used = type();
        result.original_size = ciphertext.size();
        result.final_size = plaintext.length();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

} // namespace classical
} // namespace algorithms
} // namespace filevault
