#ifndef FILEVAULT_ALGORITHMS_CLASSICAL_SUBSTITUTION_HPP
#define FILEVAULT_ALGORITHMS_CLASSICAL_SUBSTITUTION_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <array>
#include <string>

namespace filevault {
namespace algorithms {
namespace classical {

/**
 * @brief Simple substitution cipher
 * 
 * Educational cipher using a 26-letter substitution alphabet
 * Key: 26-character permutation of the alphabet
 */
class SubstitutionCipher : public core::ICryptoAlgorithm {
public:
    std::string name() const override { return "Substitution"; }
    
    core::AlgorithmType type() const override {
        return core::AlgorithmType::SUBSTITUTION;
    }
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    size_t key_size() const override { return 26; }  // 26-letter alphabet
    
    bool is_suitable_for(core::SecurityLevel level) const override {
        // Educational cipher - suitable for learning only
        (void)level;
        return false;  // Not recommended for real encryption
    }

private:
    using SubstitutionMap = std::array<char, 26>;
    
    SubstitutionMap parse_key(std::span<const uint8_t> key);
    SubstitutionMap create_reverse_map(const SubstitutionMap& forward_map);
    bool is_valid_key(const SubstitutionMap& map);
    
    std::string apply_substitution(const std::string& text, const SubstitutionMap& map);
};

} // namespace classical
} // namespace algorithms
} // namespace filevault

#endif
