#include "filevault/utils/config.hpp"
#include "filevault/utils/console.hpp"
#include <fstream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

namespace filevault {
namespace utils {

std::filesystem::path Config::get_config_path() {
    std::filesystem::path config_dir;
    
#ifdef _WIN32
    // Windows: %USERPROFILE%\.filevault\config.json
    char* user_profile = nullptr;
    size_t len = 0;
    if (_dupenv_s(&user_profile, &len, "USERPROFILE") == 0 && user_profile) {
        config_dir = std::filesystem::path(user_profile) / ".filevault";
        free(user_profile);
    }
#else
    // Unix: ~/.filevault/config.json
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        }
    }
    if (home) {
        config_dir = std::filesystem::path(home) / ".filevault";
    }
#endif
    
    // Create directory if it doesn't exist
    if (!config_dir.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(config_dir, ec);
    }
    
    return config_dir / "config.json";
}

Config Config::load() {
    auto config_path = get_config_path();
    
    if (!std::filesystem::exists(config_path)) {
        // Return default config if file doesn't exist
        return get_default();
    }
    
    try {
        std::ifstream file(config_path);
        if (!file) {
            return get_default();
        }
        
        nlohmann::json j;
        file >> j;
        
        return from_json(j);
        
    } catch (const std::exception&) {
        // If parsing fails, return default
        return get_default();
    }
}

bool Config::save() const {
    try {
        auto config_path = get_config_path();
        
        // Ensure directory exists
        auto parent = config_path.parent_path();
        if (!parent.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parent, ec);
            if (ec) {
                return false;
            }
        }
        
        std::ofstream file(config_path);
        if (!file) {
            return false;
        }
        
        auto j = to_json();
        file << j.dump(4);  // Pretty print with 4-space indent
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

Config Config::get_default() {
    Config config;
    config.default_mode_ = "standard";
    config.default_algorithm_ = "aes-256-gcm";
    config.default_kdf_ = "argon2id";
    config.default_compression_ = "none";
    config.compression_level_ = 6;
    config.show_progress_ = true;
    config.verbose_ = false;
    return config;
}

void Config::reset() {
    *this = get_default();
}

std::optional<std::string> Config::get(const std::string& key) const {
    if (key == "default.mode") return default_mode_;
    if (key == "default.algorithm") return default_algorithm_;
    if (key == "default.kdf") return default_kdf_;
    if (key == "default.compression") return default_compression_;
    if (key == "compression_level") return std::to_string(compression_level_);
    if (key == "show_progress") return show_progress_ ? "true" : "false";
    if (key == "verbose") return verbose_ ? "true" : "false";
    
    return std::nullopt;
}

bool Config::set(const std::string& key, const std::string& value) {
    if (key == "default.mode") {
        default_mode_ = value;
        return true;
    }
    if (key == "default.algorithm") {
        default_algorithm_ = value;
        return true;
    }
    if (key == "default.kdf") {
        default_kdf_ = value;
        return true;
    }
    if (key == "default.compression") {
        default_compression_ = value;
        return true;
    }
    if (key == "compression_level") {
        try {
            compression_level_ = std::stoi(value);
            return true;
        } catch (...) {
            return false;
        }
    }
    if (key == "show_progress") {
        show_progress_ = (value == "true" || value == "1" || value == "yes");
        return true;
    }
    if (key == "verbose") {
        verbose_ = (value == "true" || value == "1" || value == "yes");
        return true;
    }
    
    return false;
}

nlohmann::json Config::to_json() const {
    return nlohmann::json{
        {"version", "1.0"},
        {"default", {
            {"mode", default_mode_},
            {"algorithm", default_algorithm_},
            {"kdf", default_kdf_},
            {"compression", default_compression_}
        }},
        {"compression_level", compression_level_},
        {"ui", {
            {"show_progress", show_progress_},
            {"verbose", verbose_}
        }}
    };
}

Config Config::from_json(const nlohmann::json& j) {
    Config config;
    
    try {
        if (j.contains("default")) {
            const auto& def = j["default"];
            if (def.contains("mode")) config.default_mode_ = def["mode"];
            if (def.contains("algorithm")) config.default_algorithm_ = def["algorithm"];
            if (def.contains("kdf")) config.default_kdf_ = def["kdf"];
            if (def.contains("compression")) config.default_compression_ = def["compression"];
        }
        
        if (j.contains("compression_level")) {
            config.compression_level_ = j["compression_level"];
        }
        
        if (j.contains("ui")) {
            const auto& ui = j["ui"];
            if (ui.contains("show_progress")) config.show_progress_ = ui["show_progress"];
            if (ui.contains("verbose")) config.verbose_ = ui["verbose"];
        }
        
    } catch (const std::exception&) {
        // If any field fails, keep default value
    }
    
    return config;
}

} // namespace utils
} // namespace filevault
