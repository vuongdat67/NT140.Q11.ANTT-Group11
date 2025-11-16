#ifndef FILEVAULT_CORE_MODES_HPP
#define FILEVAULT_CORE_MODES_HPP

#include "filevault/core/types.hpp"
#include <string>

namespace filevault {
namespace core {

/**
 * @brief Mode presets for different user levels
 * 
 * BASIC: Fast, good security (student/casual use)
 * STANDARD: Balanced security/performance (professional use)
 * ADVANCED: Maximum security (paranoid/high-value data)
 */
struct ModePreset {
    UserMode mode;
    AlgorithmType algorithm;
    KDFType kdf;
    SecurityLevel security_level;
    CompressionType compression;
    int compression_level;
    
    // KDF parameters
    uint32_t kdf_iterations;
    uint32_t kdf_memory_kb;
    uint32_t kdf_parallelism;
    
    std::string name() const;
    std::string description() const;
    
    /**
     * @brief Apply preset to config
     */
    void apply_to(EncryptionConfig& config) const;
    
    /**
     * @brief Get preset by mode
     */
    static ModePreset get_preset(UserMode mode);
    
    /**
     * @brief Parse mode from string
     */
    static UserMode parse_mode(const std::string& name);
    
    /**
     * @brief Get all available presets
     */
    static std::vector<ModePreset> get_all_presets();
};

// Predefined presets
namespace presets {
    extern const ModePreset BASIC;
    extern const ModePreset STANDARD;
    extern const ModePreset ADVANCED;
}

} // namespace core
} // namespace filevault

#endif // FILEVAULT_CORE_MODES_HPP
