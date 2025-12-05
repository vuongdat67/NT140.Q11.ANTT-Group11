#include "filevault/cli/commands/config_cmd.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/table_formatter.hpp"
#include <fmt/core.h>

namespace filevault {
namespace cli {

void ConfigCommand::setup(CLI::App& app) {
    auto* config_cmd = app.add_subcommand(name(), description());
    
    // Subcommands
    auto* show = config_cmd->add_subcommand("show", "Show current configuration");
    show->callback([this]() { 
        subcommand_ = "show";
        execute();
    });
    
    auto* set = config_cmd->add_subcommand("set", "Set configuration value");
    set->add_option("key", key_, "Configuration key (e.g., default.mode)")->required();
    set->add_option("value", value_, "Value to set")->required();
    set->callback([this]() {
        subcommand_ = "set";
        execute();
    });
    
    auto* reset = config_cmd->add_subcommand("reset", "Reset to default configuration");
    reset->callback([this]() {
        subcommand_ = "reset";
        execute();
    });

    config_cmd->footer(
        "\nExamples:\n"
        "  Show config:           filevault config show\n"
        "  Set default mode:      filevault config set default.mode standard\n"
        "  Set compression level: filevault config set compression_level 9\n"
        "  Reset config:          filevault config reset\n"
        "\n"
        "Symmetric algorithms: aes-128-gcm, aes-192-gcm, aes-256-gcm, chacha20-poly1305,\n"
        "  serpent-256-gcm, twofish-{128,192,256}-gcm, camellia-{128,192,256}-gcm,\n"
        "  aria-{128,192,256}-gcm, sm4-gcm, aes-{128,192,256}-{cbc,ctr,cfb,ofb,ecb,xts}\n"
        "Asymmetric: rsa-{2048,3072,4096}, ecc-{p256,p384,p521}\n"
        "Post-Quantum: kyber-{512,768,1024}-hybrid\n"
        "Classical: caesar, vigenere, playfair, substitution, hill\n"
        "KDF options: argon2id, argon2i, pbkdf2-sha256, pbkdf2-sha512, scrypt\n"
        "Compression: none, zlib, bzip2, lzma (levels 1-9)\n"
    );
    
    config_cmd->require_subcommand(1);
}

int ConfigCommand::execute() {
    if (subcommand_ == "show") {
        return show_config();
    } else if (subcommand_ == "set") {
        return set_value();
    } else if (subcommand_ == "reset") {
        return reset_config();
    }
    
    return 1;
}

int ConfigCommand::show_config() {
    try {
        auto config = utils::Config::load();
        auto config_path = utils::Config::get_config_path();
        
        utils::Console::header("FileVault Configuration");
        
        utils::Console::info(fmt::format("Config file: {}", config_path.string()));
        utils::Console::separator();
        
        // Display settings
        fmt::print("\n");
        fmt::print("  {:25} : {}\n", "Default Mode", config.get_default_mode());
        fmt::print("  {:25} : {}\n", "Default Algorithm", config.get_default_algorithm());
        fmt::print("  {:25} : {}\n", "Default KDF", config.get_default_kdf());
        fmt::print("  {:25} : {}\n", "Default Compression", config.get_default_compression());
        fmt::print("  {:25} : {}\n", "Compression Level", config.get_compression_level());
        fmt::print("  {:25} : {}\n", "Show Progress", config.get_show_progress() ? "yes" : "no");
        fmt::print("  {:25} : {}\n", "Verbose", config.get_verbose() ? "yes" : "no");
        fmt::print("\n");
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Failed to load config: {}", e.what()));
        return 1;
    }
}

int ConfigCommand::set_value() {
    try {
        auto config = utils::Config::load();
        
        if (!config.set(key_, value_)) {
            utils::Console::error(fmt::format("Unknown configuration key: {}", key_));
            utils::Console::info("Valid keys:");
            utils::Console::info("  default.mode (basic/standard/advanced)");
            utils::Console::info("  default.algorithm (aes-256-gcm, etc.)");
            utils::Console::info("  default.kdf (argon2id, pbkdf2-sha256, etc.)");
            utils::Console::info("  default.compression (none/zlib/lzma)");
            utils::Console::info("  compression_level (1-9)");
            utils::Console::info("  show_progress (true/false)");
            utils::Console::info("  verbose (true/false)");
            return 1;
        }
        
        if (!config.save()) {
            utils::Console::error("Failed to save configuration");
            return 1;
        }
        
        utils::Console::success(fmt::format("Set {} = {}", key_, value_));
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Failed to set config: {}", e.what()));
        return 1;
    }
}

int ConfigCommand::reset_config() {
    try {
        auto config = utils::Config::get_default();
        
        if (!config.save()) {
            utils::Console::error("Failed to reset configuration");
            return 1;
        }
        
        utils::Console::success("Configuration reset to defaults");
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Failed to reset config: {}", e.what()));
        return 1;
    }
}

} // namespace cli
} // namespace filevault
