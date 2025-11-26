#include "filevault/core/types.hpp"

namespace filevault {
namespace core {

void EncryptionConfig::apply_security_level() {
    // OPTIMIZED: Lower memory for faster performance
    // Argon2 memory is THE slowest part - balance security vs usability
    switch (level) {
        case SecurityLevel::WEAK:
            kdf_iterations = 1;       // Fast for testing
            kdf_memory_kb = 4096;     // 4 MB (reasonable for tests)
            kdf_parallelism = 1;
            break;
            
        case SecurityLevel::MEDIUM:
            kdf_iterations = 2;       // Balanced - ~1-2 seconds
            kdf_memory_kb = 16384;    // 16 MB (good balance)
            kdf_parallelism = 2;
            break;
            
        case SecurityLevel::STRONG:
            kdf_iterations = 3;       // High security - ~5-10 seconds
            kdf_memory_kb = 65536;    // 64 MB (recommended by RFC)
            kdf_parallelism = 4;
            break;
            
        case SecurityLevel::PARANOID:
            kdf_iterations = 4;       // Maximum - ~20-30 seconds
            kdf_memory_kb = 131072;   // 128 MB (still reasonable)
            kdf_parallelism = 4;
            break;
    }
}

void EncryptionConfig::apply_user_mode() {
    switch (mode) {
        case UserMode::STUDENT:
            // Educational mode - classical ciphers only
            // Start with simplest cipher (Caesar)
            algorithm = AlgorithmType::CAESAR;
            kdf = KDFType::PBKDF2_SHA256;  // Simple KDF
            level = SecurityLevel::WEAK;    // Fast for learning
            compression = CompressionType::NONE;
            break;
            
        case UserMode::PROFESSIONAL:
            // Default professional settings
            // Balance between security and performance
            algorithm = AlgorithmType::AES_256_GCM;
            kdf = KDFType::ARGON2ID;
            level = SecurityLevel::MEDIUM;
            compression = CompressionType::ZLIB;  // Fast compression
            break;
            
        case UserMode::ADVANCED:
            // Maximum security for advanced users
            // No compromise on security
            algorithm = AlgorithmType::AES_256_GCM;
            kdf = KDFType::ARGON2ID;
            level = SecurityLevel::PARANOID;
            compression = CompressionType::LZMA;  // Best compression
            break;
    }
    
    // Apply the security level settings
    apply_security_level();
}

} // namespace core
} // namespace filevault
