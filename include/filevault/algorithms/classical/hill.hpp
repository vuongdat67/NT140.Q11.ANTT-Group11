#ifndef FILEVAULT_ALGORITHMS_CLASSICAL_HILL_HPP
#define FILEVAULT_ALGORITHMS_CLASSICAL_HILL_HPP

#include "filevault/core/crypto_algorithm.hpp"
#include <vector>
#include <array>

namespace filevault {
namespace algorithms {
namespace classical {

/**
 * @brief Hill cipher (2x2 matrix)
 * 
 * Educational cipher using matrix multiplication
 * Key: 4 integers forming invertible 2x2 matrix mod 26
 */
class HillCipher : public core::ICryptoAlgorithm {
public:
    std::string name() const override { return "Hill"; }
    
    core::AlgorithmType type() const override {
        return core::AlgorithmType::HILL;
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
    
    size_t key_size() const override { return 4; }  // 2x2 matrix = 4 values
    
    bool is_suitable_for(core::SecurityLevel level) const override {
        // Educational cipher - suitable for learning only
        (void)level;
        return false;  // Not recommended for real encryption
    }

private:
    using Matrix2x2 = std::array<int, 4>;  // [a,b,c,d] for matrix [[a,b],[c,d]]
    
    Matrix2x2 parse_key(std::span<const uint8_t> key);
    Matrix2x2 invert_matrix(const Matrix2x2& matrix);
    int mod_inverse(int a, int m);
    int determinant(const Matrix2x2& matrix);
    bool is_valid_key(const Matrix2x2& matrix);
    
    std::string encrypt_block(const std::string& block, const Matrix2x2& key_matrix);
    std::string decrypt_block(const std::string& block, const Matrix2x2& inv_matrix);
};

} // namespace classical
} // namespace algorithms
} // namespace filevault

#endif
