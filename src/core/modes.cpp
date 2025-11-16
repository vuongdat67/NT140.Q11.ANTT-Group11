#include "filevault/core/modes.hpp"
#include <algorithm>
#include <cctype>

namespace filevault {
namespace core {

// Predefined mode presets
namespace presets {
    const ModePreset BASIC = {
        .mode = UserMode::STUDENT,
        .algorithm = AlgorithmType::AES_256_GCM,
        .kdf = KDFType::PBKDF2_SHA256,
        .security_level = SecurityLevel::MEDIUM,
        .compression = CompressionType::ZLIB,
        .compression_level = 6,
        .kdf_iterations = 100000,      // Fast KDF
        .kdf_memory_kb = 32768,        // 32 MB
        .kdf_parallelism = 2
    };
    
    const ModePreset STANDARD = {
        .mode = UserMode::PROFESSIONAL,
        .algorithm = AlgorithmType::AES_256_GCM,
        .kdf = KDFType::ARGON2ID,
        .security_level = SecurityLevel::STRONG,
        .compression = CompressionType::ZLIB,
        .compression_level = 9,
        .kdf_iterations = 100000,      // Balanced
        .kdf_memory_kb = 65536,        // 64 MB
        .kdf_parallelism = 4
    };
    
    const ModePreset ADVANCED = {
        .mode = UserMode::ADVANCED,
        .algorithm = AlgorithmType::AES_256_GCM,
        .kdf = KDFType::ARGON2ID,
        .security_level = SecurityLevel::PARANOID,
        .compression = CompressionType::LZMA,
        .compression_level = 9,
        .kdf_iterations = 200000,      // Maximum security
        .kdf_memory_kb = 131072,       // 128 MB
        .kdf_parallelism = 8
    };
}

std::string ModePreset::name() const {
    switch (mode) {
        case UserMode::STUDENT: return "basic";
        case UserMode::PROFESSIONAL: return "standard";
        case UserMode::ADVANCED: return "advanced";
        default: return "unknown";
    }
}

std::string ModePreset::description() const {
    switch (mode) {
        case UserMode::STUDENT:
            return "Fast encryption, good security (casual use)";
        case UserMode::PROFESSIONAL:
            return "Balanced security and performance (recommended)";
        case UserMode::ADVANCED:
            return "Maximum security, slower (high-value data)";
        default:
            return "";
    }
}

void ModePreset::apply_to(EncryptionConfig& config) const {
    config.algorithm = algorithm;
    config.kdf = kdf;
    config.level = security_level;
    config.compression = compression;
    config.show_progress = true;
    config.verbose = false;
    
    // Apply security level settings
    config.apply_security_level();
    
    // Override KDF parameters with preset values
    config.kdf_iterations = kdf_iterations;
    config.kdf_memory_kb = kdf_memory_kb;
    config.kdf_parallelism = kdf_parallelism;
}

ModePreset ModePreset::get_preset(UserMode mode) {
    switch (mode) {
        case UserMode::STUDENT: return presets::BASIC;
        case UserMode::PROFESSIONAL: return presets::STANDARD;
        case UserMode::ADVANCED: return presets::ADVANCED;
        default: return presets::STANDARD;
    }
}

UserMode ModePreset::parse_mode(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "basic" || lower == "student" || lower == "fast") {
        return UserMode::STUDENT;
    }
    if (lower == "standard" || lower == "professional" || lower == "balanced") {
        return UserMode::PROFESSIONAL;
    }
    if (lower == "advanced" || lower == "paranoid" || lower == "maximum") {
        return UserMode::ADVANCED;
    }
    
    return UserMode::PROFESSIONAL;  // Default
}

std::vector<ModePreset> ModePreset::get_all_presets() {
    return {
        presets::BASIC,
        presets::STANDARD,
        presets::ADVANCED
    };
}

} // namespace core
} // namespace filevault
