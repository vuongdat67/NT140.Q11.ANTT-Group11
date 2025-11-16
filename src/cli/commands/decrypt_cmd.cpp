#include "filevault/cli/commands/decrypt_cmd.hpp"
#include "filevault/format/file_header.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/file_io.hpp"
#include "filevault/utils/crypto_utils.hpp"
#include <iostream>

namespace filevault {
namespace cli {

DecryptCommand::DecryptCommand(core::CryptoEngine& engine)
    : engine_(engine) {
}

void DecryptCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name(), description());
    
    cmd->add_option("input", input_file_, "Input encrypted file")
        ->required()
        ->check(CLI::ExistingFile);
    
    cmd->add_option("output", output_file_, "Output decrypted file");
    cmd->add_option("-p,--password", password_, "Decryption password");
    cmd->add_flag("-v,--verbose", verbose_, "Verbose output");
    
    cmd->callback([this]() { execute(); });
}

int DecryptCommand::execute() {
    try {
        utils::Console::header("FileVault Decryption");
        
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
            output_file_ = input_file_;
            // Remove .fvlt extension if present
            if (output_file_.size() > 5 && output_file_.substr(output_file_.size() - 5) == ".fvlt") {
                output_file_ = output_file_.substr(0, output_file_.size() - 5);
            } else {
                output_file_ += ".decrypted";
            }
        }
        
        utils::Console::info(fmt::format("Input:  {}", input_file_));
        utils::Console::info(fmt::format("Output: {}", output_file_));
        utils::Console::separator();
        
        // Read encrypted file
        auto file_result = utils::FileIO::read_file(input_file_);
        if (!file_result) {
            utils::Console::error(file_result.error_message);
            return 1;
        }
        
        const auto& encrypted_file = file_result.value;
        utils::Console::info(fmt::format("Read {} bytes", encrypted_file.size()));
        
        // Parse header
        auto header_result = format::FileHeader::deserialize(encrypted_file);
        if (!header_result) {
            utils::Console::error(header_result.error_message);
            return 1;
        }
        
        auto header = header_result.value;
        if (!header.validate()) {
            utils::Console::error("Invalid file header");
            return 1;
        }
        
        utils::Console::info(fmt::format("Algorithm: {}", 
                           engine_.algorithm_name(header.algorithm())));
        utils::Console::info(fmt::format("KDF:       {}", 
                           engine_.kdf_name(header.kdf())));
        
        // Get algorithm
        auto* algorithm = engine_.get_algorithm(header.algorithm());
        if (!algorithm) {
            utils::Console::error("Algorithm not supported");
            return 1;
        }
        
        // Setup config for key derivation and decryption
        core::EncryptionConfig config;
        config.algorithm = header.algorithm();
        config.kdf = header.kdf();
        config.level = header.security_level();
        config.nonce = header.nonce();
        config.tag = header.tag();
        config.apply_security_level();
        
        // Derive key from password and salt
        auto key = engine_.derive_key(password_, header.salt(), config);
        
        // Extract ciphertext (skip header)
        size_t header_size = header.total_size();
        if (encrypted_file.size() < header_size) {
            utils::Console::error("File corrupted: too small");
            return 1;
        }
        
        std::span<const uint8_t> ciphertext(
            encrypted_file.data() + header_size,
            encrypted_file.size() - header_size
        );
        
        // Decrypt
        utils::Console::info("Decrypting...");
        auto decrypt_result = algorithm->decrypt(ciphertext, key, config);
        
        if (!decrypt_result.success) {
            utils::Console::error(decrypt_result.error_message);
            if (decrypt_result.error_message.find("Authentication failed") != std::string::npos) {
                utils::Console::error("Wrong password or file corrupted/tampered");
            }
            return 1;
        }
        
        utils::Console::info(fmt::format("Decrypted in {:.2f}ms", decrypt_result.processing_time_ms));
        
        // Verify size matches
        if (decrypt_result.data.size() != header.original_size()) {
            utils::Console::warning("Decrypted size doesn't match original size");
        }
        
        // Write output
        auto write_result = utils::FileIO::write_file(output_file_, decrypt_result.data);
        if (!write_result) {
            utils::Console::error(write_result.error_message);
            return 1;
        }
        
        utils::Console::separator();
        utils::Console::success("Decryption completed!");
        utils::Console::info(fmt::format("Output: {} ({} bytes)", 
                           output_file_,
                           utils::CryptoUtils::format_bytes(decrypt_result.data.size())));
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Decryption failed: {}", e.what()));
        return 1;
    }
}

} // namespace cli
} // namespace filevault
