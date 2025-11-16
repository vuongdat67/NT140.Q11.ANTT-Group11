#include "filevault/algorithms/classical/hill.hpp"
#include <algorithm>
#include <sstream>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace classical {

HillCipher::Matrix2x2 HillCipher::parse_key(std::span<const uint8_t> key) {
    if (key.empty()) {
        return {1, 0, 0, 1};  // Identity matrix fallback
    }
    
    // Generate matrix from binary key
    // Use key bytes modulo 26, ensuring odd diagonal for invertibility
    Matrix2x2 matrix;
    for (size_t i = 0; i < 4; i++) {
        size_t idx = std::min(i, key.size() - 1);
        matrix[i] = (key[idx] + i) % 26;
    }
    
    // Make diagonal elements odd (coprime with 26 = 2*13)
    if (matrix[0] % 2 == 0 || matrix[0] == 0 || matrix[0] % 13 == 0) matrix[0] = 3;
    if (matrix[3] % 2 == 0 || matrix[3] == 0 || matrix[3] % 13 == 0) matrix[3] = 5;
    
    return matrix;
}

int HillCipher::determinant(const Matrix2x2& matrix) {
    // det([[a,b],[c,d]]) = ad - bc
    return (matrix[0] * matrix[3] - matrix[1] * matrix[2]) % 26;
}

int HillCipher::mod_inverse(int a, int m) {
    a = ((a % m) + m) % m;
    
    for (int x = 1; x < m; x++) {
        if ((a * x) % m == 1) {
            return x;
        }
    }
    
    return -1;  // No inverse exists
}

bool HillCipher::is_valid_key(const Matrix2x2& matrix) {
    int det = determinant(matrix);
    det = ((det % 26) + 26) % 26;
    
    // Determinant must be coprime with 26
    return mod_inverse(det, 26) != -1;
}

HillCipher::Matrix2x2 HillCipher::invert_matrix(const Matrix2x2& matrix) {
    int det = determinant(matrix);
    det = ((det % 26) + 26) % 26;
    
    int det_inv = mod_inverse(det, 26);
    if (det_inv == -1) {
        // Return identity if not invertible
        return {1, 0, 0, 1};
    }
    
    // Inverse = (1/det) * [[d,-b],[-c,a]]
    Matrix2x2 inv;
    inv[0] = (det_inv * matrix[3]) % 26;
    inv[1] = (det_inv * (-matrix[1])) % 26;
    inv[2] = (det_inv * (-matrix[2])) % 26;
    inv[3] = (det_inv * matrix[0]) % 26;
    
    // Ensure positive values
    for (int& val : inv) {
        val = ((val % 26) + 26) % 26;
    }
    
    return inv;
}

std::string HillCipher::encrypt_block(const std::string& block, const Matrix2x2& key_matrix) {
    if (block.length() != 2) return "";
    
    int p0 = toupper(block[0]) - 'A';
    int p1 = toupper(block[1]) - 'A';
    
    // Matrix multiplication: [c0, c1] = [a,b;c,d] * [p0; p1]
    int c0 = (key_matrix[0] * p0 + key_matrix[1] * p1) % 26;
    int c1 = (key_matrix[2] * p0 + key_matrix[3] * p1) % 26;
    
    std::string result;
    result += static_cast<char>('A' + c0);
    result += static_cast<char>('A' + c1);
    
    return result;
}

std::string HillCipher::decrypt_block(const std::string& block, const Matrix2x2& inv_matrix) {
    return encrypt_block(block, inv_matrix);  // Same operation with inverse matrix
}

core::CryptoResult HillCipher::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    (void)config;
    core::CryptoResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto key_matrix = parse_key(key);
        
        if (!is_valid_key(key_matrix)) {
            result.success = false;
            result.error_message = "Invalid key: matrix not invertible (determinant not coprime with 26)";
            return result;
        }
        
        std::string text(plaintext.begin(), plaintext.end());
        std::string ciphertext;
        
        // Remove non-alphabetic characters
        std::string cleaned;
        for (char c : text) {
            if (isalpha(c)) {
                cleaned += toupper(c);
            }
        }
        
        // Pad to even length
        if (cleaned.length() % 2 != 0) {
            cleaned += 'X';
        }
        
        // Encrypt in 2-character blocks
        for (size_t i = 0; i < cleaned.length(); i += 2) {
            std::string block = cleaned.substr(i, 2);
            ciphertext += encrypt_block(block, key_matrix);
        }
        
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

core::CryptoResult HillCipher::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    (void)config;
    core::CryptoResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        auto key_matrix = parse_key(key);
        
        if (!is_valid_key(key_matrix)) {
            result.success = false;
            result.error_message = "Invalid key: matrix not invertible";
            return result;
        }
        
        auto inv_matrix = invert_matrix(key_matrix);
        
        std::string text(ciphertext.begin(), ciphertext.end());
        std::string plaintext;
        
        // Decrypt in 2-character blocks
        for (size_t i = 0; i < text.length(); i += 2) {
            if (i + 1 < text.length()) {
                std::string block = text.substr(i, 2);
                plaintext += decrypt_block(block, inv_matrix);
            }
        }
        
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
