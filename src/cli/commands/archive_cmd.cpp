#include "filevault/cli/commands/archive_cmd.hpp"
#include "filevault/archive/archive_format.hpp"
#include "filevault/compression/compressor.hpp"
#include "filevault/utils/file_io.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/password.hpp"
#include "filevault/core/file_format.hpp"
#include <fmt/core.h>
#include <filesystem>
#include <chrono>

namespace filevault::cli::commands {

namespace fs = std::filesystem;

void ArchiveCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name(), description());
    
    // Create mode
    auto* create_cmd = cmd->add_subcommand("create", "Create encrypted archive");
    create_cmd->add_option("files", input_files_, "Files to archive")
        ->required()
        ->check(CLI::ExistingFile);
    create_cmd->add_option("-o,--output", output_file_, "Output archive file")
        ->required();
    create_cmd->add_option("-p,--password", password_, "Encryption password");
    create_cmd->add_option("-a,--algorithm", algorithm_, "Encryption algorithm")
        ->check(CLI::IsMember({"aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-poly1305"}));
    create_cmd->add_option("-c,--compression", compression_, "Compression algorithm")
        ->check(CLI::IsMember({"zlib", "bzip2", "lzma", "none"}));
    create_cmd->add_option("-k,--kdf", kdf_, "Key derivation function")
        ->check(CLI::IsMember({"argon2id", "argon2i", "pbkdf2-sha256", "pbkdf2-sha512"}));
    create_cmd->add_option("-s,--security", security_level_, "Security level")
        ->check(CLI::IsMember({"weak", "medium", "strong", "paranoid"}));
    create_cmd->add_flag("-v,--verbose", verbose_, "Verbose output");
    create_cmd->callback([this]() { execute(); });
    
    // Extract mode
    auto* extract_cmd = cmd->add_subcommand("extract", "Extract encrypted archive");
    extract_cmd->add_option("archive", input_files_, "Archive file to extract")
        ->required()
        ->check(CLI::ExistingFile);
    extract_cmd->add_option("-o,--output", extract_dir_, "Output directory");
    extract_cmd->add_option("-p,--password", password_, "Decryption password");
    extract_cmd->add_flag("-v,--verbose", verbose_, "Verbose output");
    extract_cmd->callback([this]() { extract_ = true; execute(); });
    
    // List mode - TODO: Implement proper listing without extraction
    auto* list_cmd = cmd->add_subcommand("list", "List archive contents");
    list_cmd->add_option("archive", input_files_, "Archive file")
        ->required()
        ->check(CLI::ExistingFile);
    list_cmd->add_option("-p,--password", password_, "Decryption password");
    list_cmd->callback([this]() { extract_ = true; execute(); });  // Temporarily use extract
    
    cmd->require_subcommand(1);
}

int ArchiveCommand::execute() {
    if (extract_) {
        return do_extract();
    } else {
        return do_create();
    }
}

int ArchiveCommand::do_create() {
    utils::Console::separator();
    fmt::print("\n{:^80}\n", "FileVault Archive Creation");
    utils::Console::separator();
    
    // Validate input files
    std::vector<fs::path> file_paths;
    for (const auto& file : input_files_) {
        if (!fs::exists(file)) {
            utils::Console::error(fmt::format("File not found: {}", file));
            return 1;
        }
        file_paths.push_back(file);
    }
    
    utils::Console::info(fmt::format("Files:       {} file(s)", file_paths.size()));
    utils::Console::info(fmt::format("Algorithm:   {}", algorithm_));
    utils::Console::info(fmt::format("Compression: {}", compression_));
    utils::Console::info(fmt::format("KDF:         {}", kdf_));
    utils::Console::info(fmt::format("Security:    {}", security_level_));
    utils::Console::separator();
    
    // Show file list
    if (verbose_) {
        utils::Console::info("Input files:");
        for (const auto& path : file_paths) {
            auto size = fs::file_size(path);
            utils::Console::info(fmt::format("  {} ({} bytes)", path.filename().string(), size));
        }
        utils::Console::separator();
    }
    
    // Step 1: Create archive
    utils::Console::info("Creating archive...");
    auto start_archive = std::chrono::high_resolution_clock::now();
    
    std::vector<uint8_t> archive_data;
    try {
        archive_data = archive::ArchiveFormat::create_archive(file_paths);
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Archive creation failed: {}", e.what()));
        return 1;
    }
    
    auto end_archive = std::chrono::high_resolution_clock::now();
    auto archive_time = std::chrono::duration<double, std::milli>(end_archive - start_archive);
    
    utils::Console::success(fmt::format("Archive created ({} bytes)", archive_data.size()));
    
    // Step 2: Compress (if requested)
    std::vector<uint8_t> compressed_data;
    if (compression_ != "none") {
        utils::Console::info(fmt::format("Compressing with {}...", compression_));
        auto start_compress = std::chrono::high_resolution_clock::now();
        
        auto comp_type = compression::CompressionService::parse_algorithm(compression_);
        auto compressor = compression::CompressionService::create(comp_type);
        
        if (!compressor) {
            utils::Console::error("Failed to create compressor");
            return 1;
        }
        
        auto compress_result = compressor->compress(archive_data, 6);
        if (!compress_result.success) {
            utils::Console::error(compress_result.error_message);
            return 1;
        }
        
        compressed_data = std::move(compress_result.data);
        auto end_compress = std::chrono::high_resolution_clock::now();
        auto compress_time = std::chrono::duration<double, std::milli>(end_compress - start_compress);
        
        double ratio = (double)compressed_data.size() / archive_data.size() * 100.0;
        utils::Console::success(fmt::format("Compressed to {} bytes ({:.1f}%)", 
                                           compressed_data.size(), ratio));
        
        if (verbose_) {
            utils::Console::info(fmt::format("Compression time: {:.2f} ms", compress_time.count()));
        }
    } else {
        compressed_data = archive_data;
        utils::Console::info("Skipping compression");
    }
    
    // Step 3: Encrypt
    utils::Console::info("Encrypting...");
    
    // Get password
    if (password_.empty()) {
        password_ = utils::Password::read_secure("Enter password for archive: ", true);
        if (password_.empty()) {
            utils::Console::error("Password required");
            return 1;
        }
    }
    
    auto start_encrypt = std::chrono::high_resolution_clock::now();
    
    // Parse algorithm
    auto algo_type_opt = engine_.parse_algorithm(algorithm_);
    if (!algo_type_opt) {
        utils::Console::error(fmt::format("Unknown algorithm: {}", algorithm_));
        return 1;
    }
    auto algo_type = algo_type_opt.value();
    
    // Parse KDF and security level
    auto kdf_type_opt = engine_.parse_kdf(kdf_);
    auto sec_level_opt = engine_.parse_security_level(security_level_);
    
    if (!kdf_type_opt || !sec_level_opt) {
        utils::Console::error("Invalid KDF or security level");
        return 1;
    }
    
    auto kdf_type = kdf_type_opt.value();
    auto sec_level = sec_level_opt.value();
    
    // Setup encryption config
    core::EncryptionConfig config;
    config.algorithm = algo_type;
    config.kdf = kdf_type;
    config.level = sec_level;
    config.apply_security_level();
    config.compression = compression_ != "none" ? core::CompressionType::ZLIB : core::CompressionType::NONE;
    
    // Generate salt and derive key
    auto salt = engine_.generate_salt(32);
    auto key = engine_.derive_key(password_, salt, config);
    auto nonce = engine_.generate_nonce(12);
    config.nonce = nonce;
    
    // Get algorithm and encrypt
    auto* algorithm = engine_.get_algorithm(algo_type);
    if (!algorithm) {
        utils::Console::error("Failed to get algorithm");
        return 1;
    }
    
    auto encrypt_result = algorithm->encrypt(compressed_data, key, config);
    
    auto end_encrypt = std::chrono::high_resolution_clock::now();
    auto encrypt_time = std::chrono::duration<double, std::milli>(end_encrypt - start_encrypt);
    
    if (!encrypt_result.success) {
        utils::Console::error(encrypt_result.error_message);
        return 1;
    }
    
    utils::Console::success(fmt::format("Encrypted ({} bytes)", encrypt_result.data.size()));
    
    // Create file header and write encrypted file
    auto header = core::FileFormatHandler::create_header(
        algo_type, kdf_type, config, salt, nonce, compression_ != "none"
    );
    
    // Get auth tag for AEAD algorithms
    std::vector<uint8_t> auth_tag;
    if (encrypt_result.tag.has_value()) {
        auth_tag = encrypt_result.tag.value();
    }
    
    // Write encrypted archive
    core::FileFormatHandler::write_file(
        output_file_, header, encrypt_result.data, auth_tag
    );
    
    std::vector<uint8_t> final_data; // Just for size reporting
    auto file_result = utils::FileIO::read_file(output_file_);
    if (file_result.success) {
        final_data = file_result.value;
    }
    
    // Summary
    utils::Console::separator();
    utils::Console::success("Archive created successfully!");
    utils::Console::info(fmt::format("Output:     {}", output_file_));
    utils::Console::info(fmt::format("Size:       {} bytes", final_data.size()));
    
    if (verbose_) {
        auto total_time = archive_time.count() + encrypt_time.count();
        if (compression_ != "none") {
            // Add compression time if we tracked it
        }
        utils::Console::info(fmt::format("Total time: {:.2f} ms", total_time));
    }
    
    return 0;
}

int ArchiveCommand::do_extract() {
    utils::Console::separator();
    fmt::print("\n{:^80}\n", "FileVault Archive Extraction");
    utils::Console::separator();
    
    if (input_files_.empty()) {
        utils::Console::error("No archive file specified");
        return 1;
    }
    
    std::string archive_file = input_files_[0];
    utils::Console::info(fmt::format("Archive: {}", archive_file));
    utils::Console::info(fmt::format("Output:  {}", extract_dir_));
    utils::Console::separator();
    
    // Step 1: Read encrypted archive
    auto file_result = utils::FileIO::read_file(archive_file);
    if (!file_result.success) {
        utils::Console::error(file_result.error_message);
        return 1;
    }
    
    auto encrypted_data = file_result.value;
    utils::Console::info(fmt::format("Read {} bytes", encrypted_data.size()));
    
    // Step 2: Decrypt
    if (password_.empty()) {
        password_ = utils::Password::read_secure("Enter archive password: ", false);
        if (password_.empty()) {
            utils::Console::error("Password required");
            return 1;
        }
    }
    
    utils::Console::info("Decrypting...");
    
    // Read encrypted file format
    auto [header, ciphertext, auth_tag] = core::FileFormatHandler::read_file(archive_file);
    
    // Parse algorithm and KDF from header
    auto algo_type = core::FileFormatHandler::from_algorithm_id(header.algorithm);
    auto kdf_type = core::FileFormatHandler::from_kdf_id(header.kdf);
    
    if (verbose_) {
        utils::Console::info(fmt::format("Algorithm: {}", engine_.algorithm_name(algo_type)));
        utils::Console::info(fmt::format("KDF: {}", engine_.kdf_name(kdf_type)));
        utils::Console::info(fmt::format("Header size: {} bytes", header.size()));
        utils::Console::info(fmt::format("Ciphertext: {} bytes", ciphertext.size()));
        utils::Console::info(fmt::format("Auth tag: {} bytes", auth_tag.size()));
    }
    
    // Setup config
    core::EncryptionConfig config;
    config.algorithm = algo_type;
    config.kdf = kdf_type;
    config.nonce = header.nonce;
    config.tag = auth_tag;  // Set auth tag to config for AEAD decryption
    
    // Parse KDF params from header and set to config
    if (kdf_type == core::KDFType::ARGON2ID || kdf_type == core::KDFType::ARGON2I) {
        auto params = core::Argon2Params::deserialize(header.kdf_params);
        config.kdf_memory_kb = params.memory_kb;
        config.kdf_iterations = params.iterations;
        config.kdf_parallelism = params.parallelism;
        
        if (verbose_) {
            utils::Console::info(fmt::format("Argon2 params: memory={}KB, iterations={}, parallelism={}",
                                           params.memory_kb, params.iterations, params.parallelism));
        }
    } else if (kdf_type == core::KDFType::PBKDF2_SHA256 || kdf_type == core::KDFType::PBKDF2_SHA512) {
        auto params = core::PBKDF2Params::deserialize(header.kdf_params);
        config.kdf_iterations = params.iterations;
        
        if (verbose_) {
            utils::Console::info(fmt::format("PBKDF2 params: iterations={}", params.iterations));
        }
    }
    
    // Derive key
    if (verbose_) {
        utils::Console::info("Deriving key from password...");
    }
    auto key = engine_.derive_key(password_, header.salt, config);
    
    if (verbose_) {
        utils::Console::info(fmt::format("Key derived ({} bytes)", key.size()));
    }
    
    // Get algorithm and decrypt
    auto* algorithm = engine_.get_algorithm(algo_type);
    if (!algorithm) {
        utils::Console::error("Failed to get algorithm");
        return 1;
    }
    
    // Decrypt (nonce and tag are already in config)
    auto decrypt_result = algorithm->decrypt(ciphertext, key, config);
    
    if (!decrypt_result.success) {
        // Improved error reporting
        if (decrypt_result.error_message.empty()) {
            utils::Console::error("Decryption failed - possible causes:");
            utils::Console::error("  • Wrong password");
            utils::Console::error("  • Corrupted file");
            utils::Console::error("  • File format mismatch");
            utils::Console::error("\nTry:");
            utils::Console::error("  1. Verify password is correct");
            utils::Console::error("  2. Check if file was created with same version");
            utils::Console::error("  3. Use --verbose flag for more details");
        } else {
            utils::Console::error(fmt::format("Decryption failed: {}", decrypt_result.error_message));
        }
        return 1;
    }
    
    if (verbose_) {
        utils::Console::info(fmt::format("Decryption successful ({} bytes plaintext)", decrypt_result.data.size()));
    }
    
    auto decrypted_data = decrypt_result.data;
    utils::Console::success(fmt::format("Decrypted ({} bytes)", decrypted_data.size()));
    
    // Step 3: Decompress (auto-detect)
    std::vector<uint8_t> archive_data;
    
    // Try to detect if it's compressed by checking magic bytes
    if (decrypted_data.size() >= 2) {
        bool is_compressed = false;
        std::string compression_type;
        
        // zlib
        if (decrypted_data[0] == 0x78 && 
            (decrypted_data[1] == 0x9C || decrypted_data[1] == 0x01 || decrypted_data[1] == 0xDA)) {
            is_compressed = true;
            compression_type = "zlib";
        }
        // bzip2
        else if (decrypted_data[0] == 0x42 && decrypted_data[1] == 0x5A) {
            is_compressed = true;
            compression_type = "bzip2";
        }
        // lzma/xz
        else if (decrypted_data.size() >= 4 && 
                 decrypted_data[0] == 0xFD && decrypted_data[1] == 0x37 && 
                 decrypted_data[2] == 0x7A && decrypted_data[3] == 0x58) {
            is_compressed = true;
            compression_type = "lzma";
        }
        
        if (is_compressed) {
            utils::Console::info(fmt::format("Decompressing (detected: {})...", compression_type));
            
            auto comp_type = compression::CompressionService::parse_algorithm(compression_type);
            auto compressor = compression::CompressionService::create(comp_type);
            
            if (!compressor) {
                utils::Console::error("Failed to create decompressor");
                return 1;
            }
            
            auto decompress_result = compressor->decompress(decrypted_data);
            if (!decompress_result.success) {
                utils::Console::error(decompress_result.error_message);
                return 1;
            }
            
            archive_data = std::move(decompress_result.data);
            utils::Console::success(fmt::format("Decompressed ({} bytes)", archive_data.size()));
        } else {
            archive_data = decrypted_data;
        }
    } else {
        archive_data = decrypted_data;
    }
    
    // Step 4: Extract archive
    utils::Console::info("Extracting files...");
    
    bool success = archive::ArchiveFormat::extract_archive(archive_data, extract_dir_);
    if (!success) {
        utils::Console::error("Failed to extract archive");
        return 1;
    }
    
    // Show extracted files
    auto entries = archive::ArchiveFormat::list_files(archive_data);
    
    utils::Console::separator();
    utils::Console::success(fmt::format("Extracted {} file(s) to {}", entries.size(), extract_dir_));
    
    if (verbose_) {
        utils::Console::info("\nExtracted files:");
        for (const auto& entry : entries) {
            utils::Console::info(fmt::format("  {} ({} bytes)", entry.filename, entry.file_size));
        }
    }
    
    return 0;
}

} // namespace filevault::cli::commands

// Commented out do_list() - to be implemented later
/*
int ArchiveCommand::do_list() {
    utils::Console::separator();
    fmt::print("\n{:^80}\n", "FileVault Archive Contents");
    utils::Console::separator();
    
    if (input_files_.empty()) {
        utils::Console::error("No archive file specified");
        return 1;
    }
    
    auto archive_file = input_files_[0];
    
    if (!fs::exists(archive_file)) {
        utils::Console::error(fmt::format("Archive not found: {}", archive_file));
        return 1;
    }
    
    utils::Console::info(fmt::format("Archive: {}", archive_file));
    utils::Console::separator();
    
    // Read archive file
    auto file_result = utils::FileIO::read_binary_file(archive_file);
    if (!file_result.has_value()) {
        utils::Console::error(fmt::format("Failed to read archive: {}", file_result.error()));
        return 1;
    }
    
    auto encrypted_data = file_result.value;
    utils::Console::info(fmt::format("Read {} bytes", encrypted_data.size()));
    
    // Get password
    if (password_.empty()) {
        password_ = utils::Password::read_secure("Enter archive password: ", false);
        if (password_.empty()) {
            utils::Console::error("Password required");
            return 1;
        }
    }
    
    utils::Console::info("Decrypting...");
    
    // Read encrypted file format
    auto [header, ciphertext, auth_tag] = core::FileFormatHandler::read_file(archive_file);
    
    // Parse algorithm and KDF from header
    auto algo_type = core::FileFormatHandler::from_algorithm_id(header.algorithm);
    auto kdf_type = core::FileFormatHandler::from_kdf_id(header.kdf);
    
    if (verbose_) {
        utils::Console::info(fmt::format("Algorithm: {}", engine_.algorithm_name(algo_type)));
        utils::Console::info(fmt::format("KDF: {}", engine_.kdf_name(kdf_type)));
    }
    
    // Setup config
    core::EncryptionConfig config;
    config.algorithm = algo_type;
    config.kdf = kdf_type;
    config.nonce = header.nonce;
    config.tag = auth_tag;
    
    // Parse KDF params
    if (kdf_type == core::KDFType::ARGON2ID || kdf_type == core::KDFType::ARGON2I) {
        auto params = core::Argon2Params::deserialize(header.kdf_params);
        config.kdf_memory_kb = params.memory_kb;
        config.kdf_iterations = params.iterations;
        config.kdf_parallelism = params.parallelism;
    } else if (kdf_type == core::KDFType::PBKDF2_SHA256 || kdf_type == core::KDFType::PBKDF2_SHA512) {
        auto params = core::PBKDF2Params::deserialize(header.kdf_params);
        config.kdf_iterations = params.iterations;
    }
    
    // Derive key
    auto key = engine_.derive_key(password_, header.salt, config);
    
    // Decrypt
    auto* algorithm = engine_.get_algorithm(algo_type);
    if (!algorithm) {
        utils::Console::error("Algorithm not available");
        return 1;
    }
    
    auto decrypt_result = algorithm->decrypt(ciphertext, key, config);
    if (!decrypt_result.success) {
        utils::Console::error(decrypt_result.error_message);
        return 1;
    }
    
    auto decrypted_data = decrypt_result.data;
    utils::Console::success(fmt::format("Decrypted ({} bytes)", decrypted_data.size()));
    
    // Decompress if needed
    auto decompressed_data = decrypted_data;
    auto compression_type = core::FileFormatHandler::from_compression_id(header.compression);
    
    if (compression_type != core::CompressionType::NONE) {
        std::string compression_name = "zlib";
        if (compression_type == core::CompressionType::ZLIB) compression_name = "zlib";
        else if (compression_type == core::CompressionType::LZMA) compression_name = "lzma";
        else if (compression_type == core::CompressionType::BZIP2) compression_name = "bzip2";
        
        utils::Console::info(fmt::format("Decompressing ({})...", compression_name));
        
        auto decompressor = compression::ICompressor::create(compression_name);
        if (!decompressor) {
            utils::Console::error("Decompression not available");
            return 1;
        }
        
        auto decompress_result = decompressor->decompress(decrypted_data);
        if (!decompress_result.success) {
            utils::Console::error(decompress_result.error_message);
            return 1;
        }
        
        decompressed_data = decompress_result.data;
        utils::Console::success(fmt::format("Decompressed ({} bytes)", decompressed_data.size()));
    }
    
    // Parse archive
    auto entries = archive::ArchiveFormat::parse(decompressed_data);
    if (entries.empty()) {
        utils::Console::error("Failed to parse archive or archive is empty");
        return 1;
    }
    
    utils::Console::separator();
    utils::Console::success(fmt::format("Archive contains {} file(s):\n", entries.size()));
    
    // Print file list with details
    size_t total_size = 0;
    for (const auto& entry : entries) {
        total_size += entry.file_size;
        fmt::print("  {:40} {:>12} bytes\n", entry.filename, entry.file_size);
    }
    
    utils::Console::separator();
    fmt::print("Total: {} file(s), {} bytes\n", entries.size(), total_size);
    
    return 0;
}
*/
