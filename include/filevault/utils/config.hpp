#ifndef FILEVAULT_UTILS_CONFIG_HPP
#define FILEVAULT_UTILS_CONFIG_HPP

#include "filevault/core/types.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <filesystem>

namespace filevault {
namespace utils {

/**
 * @brief Configuration manager for FileVault
 * 
 * Manages user preferences stored in ~/.filevault/config.json
 */
class Config {
public:
    /**
     * @brief Load configuration from file
     */
    static Config load();
    
    /**
     * @brief Save configuration to file
     */
    bool save() const;
    
    /**
     * @brief Get config file path
     */
    static std::filesystem::path get_config_path();
    
    /**
     * @brief Get default config
     */
    static Config get_default();
    
    /**
     * @brief Reset to defaults
     */
    void reset();
    
    // Getters
    std::string get_default_mode() const { return default_mode_; }
    std::string get_default_algorithm() const { return default_algorithm_; }
    std::string get_default_kdf() const { return default_kdf_; }
    std::string get_default_compression() const { return default_compression_; }
    int get_compression_level() const { return compression_level_; }
    bool get_show_progress() const { return show_progress_; }
    bool get_verbose() const { return verbose_; }
    
    // Setters
    void set_default_mode(const std::string& mode) { default_mode_ = mode; }
    void set_default_algorithm(const std::string& algo) { default_algorithm_ = algo; }
    void set_default_kdf(const std::string& kdf) { default_kdf_ = kdf; }
    void set_default_compression(const std::string& comp) { default_compression_ = comp; }
    void set_compression_level(int level) { compression_level_ = level; }
    void set_show_progress(bool show) { show_progress_ = show; }
    void set_verbose(bool verbose) { verbose_ = verbose; }
    
    /**
     * @brief Get value by key path (e.g., "default.mode")
     */
    std::optional<std::string> get(const std::string& key) const;
    
    /**
     * @brief Set value by key path
     */
    bool set(const std::string& key, const std::string& value);
    
    /**
     * @brief Convert to JSON
     */
    nlohmann::json to_json() const;
    
    /**
     * @brief Load from JSON
     */
    static Config from_json(const nlohmann::json& j);

private:
    // Default settings
    std::string default_mode_ = "standard";
    std::string default_algorithm_ = "aes-256-gcm";
    std::string default_kdf_ = "argon2id";
    std::string default_compression_ = "none";
    int compression_level_ = 6;
    
    // UI preferences
    bool show_progress_ = true;
    bool verbose_ = false;
};

} // namespace utils
} // namespace filevault

#endif // FILEVAULT_UTILS_CONFIG_HPP
