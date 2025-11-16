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

} // namespace core
} // namespace filevault
