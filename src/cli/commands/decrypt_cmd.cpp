#include "filevault/cli/commands/decrypt_cmd.hpp"
#include "filevault/format/file_header.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/file_io.hpp"
#include "filevault/utils/crypto_utils.hpp"
#include "filevault/utils/password.hpp"
#include "filevault/utils/progress.hpp"
#include "filevault/compression/compressor.hpp"
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
    cmd->add_option("-p,--password", password_, "Decryption password (not recommended)");
    cmd->add_flag("-v,--verbose", verbose_, "Verbose output");
    cmd->add_flag("--no-progress", no_progress_, "Disable progress bars");
    
    cmd->callback([this]() { execute(); });
}

int DecryptCommand::execute() {
    try {
        utils::Console::header("FileVault Decryption");
        
        // Get password securely if not provided
        if (password_.empty()) {
            password_ = utils::Password::read_secure("Enter decryption password: ", false);
            if (password_.empty()) {
                utils::Console::error("Password cannot be empty");
                return 1;
            }
        } else {
            utils::Console::warning("Using password from command line is insecure!");
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
        
        // Step 1: Derive key from password and salt
        utils::Console::info("Deriving key...");
        std::unique_ptr<utils::ProgressBar> kdf_progress;
        if (!no_progress_) {
            kdf_progress = std::make_unique<utils::ProgressBar>("Deriving key", 100);
            kdf_progress->set_progress(50);
        }
        
        auto key = engine_.derive_key(password_, header.salt(), config);
        
        if (kdf_progress) {
            kdf_progress->mark_as_completed();
        }
        
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
        
        // Step 2: Decrypt
        utils::Console::info("Decrypting...");
        std::unique_ptr<utils::ProgressBar> decrypt_progress;
        if (!no_progress_) {
            decrypt_progress = std::make_unique<utils::ProgressBar>("Decrypting", 100);
            decrypt_progress->set_progress(50);
        }
        
        auto decrypt_result = algorithm->decrypt(ciphertext, key, config);
        
        if (decrypt_progress) {
            decrypt_progress->mark_as_completed();
        }
        
        if (!decrypt_result.success) {
            utils::Console::error(decrypt_result.error_message);
            if (decrypt_result.error_message.find("Authentication failed") != std::string::npos) {
                utils::Console::error("Wrong password or file corrupted/tampered");
            }
            return 1;
        }
        
        utils::Console::info(fmt::format("Decrypted in {:.2f}ms", decrypt_result.processing_time_ms));
        
        // Step 3: Decompress if needed
        auto plaintext = decrypt_result.data;
        
        if (header.is_compressed()) {
            utils::Console::info("Decompressing...");
            std::unique_ptr<utils::ProgressBar> decompress_progress;
            if (!no_progress_) {
                decompress_progress = std::make_unique<utils::ProgressBar>("Decompressing", 100);
                decompress_progress->set_progress(50);
            }
            
            // Determine compression type from header (for now assume zlib/lzma based on size)
            // TODO: Add compression type to file header
            auto decompressor = compression::CompressionService::create(core::CompressionType::LZMA);
            if (!decompressor) {
                decompressor = compression::CompressionService::create(core::CompressionType::ZLIB);
            }
            
            if (decompressor) {
                auto decompress_result = decompressor->decompress(plaintext);
                if (decompress_result.success) {
                    plaintext = std::move(decompress_result.data);
                    utils::Console::info(fmt::format("Decompressed: {} -> {} bytes",
                                       decrypt_result.data.size(),
                                       plaintext.size()));
                } else {
                    // Try other compressor
                    auto decompressor2 = compression::CompressionService::create(core::CompressionType::ZLIB);
                    if (decompressor2) {
                        auto result2 = decompressor2->decompress(decrypt_result.data);
                        if (result2.success) {
                            plaintext = std::move(result2.data);
                            utils::Console::info(fmt::format("Decompressed: {} -> {} bytes",
                                               decrypt_result.data.size(),
                                               plaintext.size()));
                        } else {
                            utils::Console::warning("Decompression failed, using raw decrypted data");
                        }
                    }
                }
            }
            
            if (decompress_progress) {
                decompress_progress->mark_as_completed();
            }
        }
        
        // Verify size matches expected original
        if (plaintext.size() != header.original_size()) {
            utils::Console::warning(fmt::format("Output size ({}) doesn't match expected size ({})",
                                   plaintext.size(), header.original_size()));
        }
        
        // Write output
        auto write_result = utils::FileIO::write_file(output_file_, plaintext);
        if (!write_result) {
            utils::Console::error(write_result.error_message);
            return 1;
        }
        
        utils::Console::separator();
        utils::Console::success("Decryption completed!");
        utils::Console::info(fmt::format("Output: {} ({})", 
                           output_file_,
                           utils::CryptoUtils::format_bytes(plaintext.size())));
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Decryption failed: {}", e.what()));
        return 1;
    }
}

} // namespace cli
} // namespace filevault
