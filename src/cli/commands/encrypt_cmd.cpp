#include "filevault/cli/commands/encrypt_cmd.hpp"
#include "filevault/format/file_header.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/file_io.hpp"
#include "filevault/utils/crypto_utils.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <iostream>

namespace filevault {
namespace cli {

EncryptCommand::EncryptCommand(core::CryptoEngine& engine)
    : engine_(engine) {
}

void EncryptCommand::setup(CLI::App& app) {
    auto* encrypt_cmd = app.add_subcommand(name(), description());
    
    encrypt_cmd->add_option("input", input_file_, "Input file to encrypt")
        ->required()
        ->check(CLI::ExistingFile);
    
    encrypt_cmd->add_option("output", output_file_, "Output encrypted file");
    
    encrypt_cmd->add_option("-a,--algorithm", algorithm_, "Encryption algorithm")
        ->check(CLI::IsMember({
            "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", 
            "chacha20-poly1305",
            "caesar", "vigenere", "playfair"
        }));
    
    encrypt_cmd->add_option("-s,--security", security_level_, "Security level")
        ->check(CLI::IsMember({"weak", "medium", "strong", "paranoid"}));
    
    encrypt_cmd->add_option("-k,--kdf", kdf_, "Key derivation function")
        ->check(CLI::IsMember({
            "argon2id", "argon2i", "pbkdf2-sha256", "pbkdf2-sha512", "scrypt"
        }));
    
    encrypt_cmd->add_option("-p,--password", password_, "Encryption password");
    
    encrypt_cmd->add_flag("-c,--compress", compress_, "Compress before encryption");
    
    encrypt_cmd->add_flag("-v,--verbose", verbose_, "Verbose output");
    
    encrypt_cmd->callback([this]() { execute(); });
}

int EncryptCommand::execute() {
    try {
        utils::Console::header("FileVault Encryption");
        
        // Get password if not provided
        if (password_.empty()) {
            std::cout << "Enter password: ";
            std::getline(std::cin, password_);
            if (password_.empty()) {
                utils::Console::error("Password cannot be empty");
                return 1;
            }
        }
        
        // Set output file if not specified
        if (output_file_.empty()) {
            output_file_ = input_file_ + ".fvlt";
        }
        
        // Show configuration
        utils::Console::info(fmt::format("Input:     {}", input_file_));
        utils::Console::info(fmt::format("Output:    {}", output_file_));
        utils::Console::info(fmt::format("Algorithm: {}", algorithm_));
        utils::Console::info(fmt::format("Security:  {}", security_level_));
        utils::Console::info(fmt::format("KDF:       {}", kdf_));
        utils::Console::separator();
        
        // Read input file
        auto file_result = utils::FileIO::read_file(input_file_);
        if (!file_result) {
            utils::Console::error(file_result.error_message);
            return 1;
        }
        
        const auto& plaintext = file_result.value;
        utils::Console::info(fmt::format("Read {} bytes", plaintext.size()));
        
        spdlog::debug("Parsing configuration...");
        // Parse configuration
        auto algo_type_opt = engine_.parse_algorithm(algorithm_);
        auto kdf_type_opt = engine_.parse_kdf(kdf_);
        auto sec_level_opt = engine_.parse_security_level(security_level_);
        
        spdlog::debug("algo_type_opt: {}, kdf_type_opt: {}, sec_level_opt: {}", 
                     algo_type_opt.has_value(), kdf_type_opt.has_value(), sec_level_opt.has_value());
        
        if (!algo_type_opt || !kdf_type_opt || !sec_level_opt) {
            utils::Console::error("Invalid configuration parameters");
            return 1;
        }
        
        auto algo_type = algo_type_opt.value();
        auto kdf_type = kdf_type_opt.value();
        auto sec_level = sec_level_opt.value();
        
        // Get algorithm
        auto* algorithm = engine_.get_algorithm(algo_type);
        if (!algorithm) {
            utils::Console::error(fmt::format("Algorithm '{}' not available", algorithm_));
            return 1;
        }
        
        // Setup encryption config
        core::EncryptionConfig config;
        config.algorithm = algo_type;
        config.kdf = kdf_type;
        config.level = sec_level;
        config.apply_security_level();
        
        // Generate salt and derive key
        auto salt = engine_.generate_salt(32);
        auto key = engine_.derive_key(password_, salt, config);
        
        // Generate nonce and add to config
        auto nonce = engine_.generate_nonce(12); // GCM standard
        config.nonce = nonce;
        
        // Encrypt
        utils::Console::info("Encrypting...");
        auto encrypt_result = algorithm->encrypt(plaintext, key, config);
        
        if (!encrypt_result.success) {
            utils::Console::error(encrypt_result.error_message);
            return 1;
        }
        
        utils::Console::info(fmt::format("Encrypted in {:.2f}ms", encrypt_result.processing_time_ms));
        
        // Create file header
        format::FileHeader header;
        header.set_algorithm(algo_type);
        header.set_kdf(kdf_type);
        header.set_security_level(sec_level);
        header.set_salt(salt);
        header.set_nonce(nonce);
        if (encrypt_result.tag.has_value()) {
            header.set_tag(encrypt_result.tag.value());
        }
        header.set_original_size(plaintext.size());
        header.set_encrypted_size(encrypt_result.data.size());
        header.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        header.set_compressed(compress_);
        
        if (!header.validate()) {
            utils::Console::error("Invalid header generated");
            return 1;
        }
        
        // Serialize header + ciphertext
        auto header_bytes = header.serialize();
        std::vector<uint8_t> output;
        output.reserve(header_bytes.size() + encrypt_result.data.size());
        output.insert(output.end(), header_bytes.begin(), header_bytes.end());
        output.insert(output.end(), encrypt_result.data.begin(), encrypt_result.data.end());
        
        // Write output file
        auto write_result = utils::FileIO::write_file(output_file_, output);
        if (!write_result) {
            utils::Console::error(write_result.error_message);
            return 1;
        }
        
        utils::Console::separator();
        utils::Console::success("Encryption completed!");
        utils::Console::info(fmt::format("Output: {} ({} bytes)", 
                           output_file_, 
                           utils::CryptoUtils::format_bytes(output.size())));
        utils::Console::info(fmt::format("Compression: {:.1f}%", 
                           100.0 * output.size() / plaintext.size()));
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Encryption failed: {}", e.what()));
        return 1;
    }
}

} // namespace cli
} // namespace filevault
