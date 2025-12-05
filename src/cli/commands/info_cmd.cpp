#include "filevault/cli/commands/info_cmd.hpp"
#include "filevault/core/file_format.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/crypto_utils.hpp"
#include <fmt/core.h>
#include <fstream>

namespace filevault {
namespace cli {

InfoCommand::InfoCommand(core::CryptoEngine& engine)
    : engine_(engine) {
}

void InfoCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name(), description());
    
    cmd->add_option("input", input_file_, "Encrypted file to inspect")
        ->required()
        ->check(CLI::ExistingFile);
    
    cmd->add_flag("-v,--verbose", verbose_, "Show detailed information");

    cmd->footer(
        "\nExamples:\n"
        "  Show file info:        filevault info secret.fvlt\n"
        "  Verbose output:       filevault info secret.fvlt -v\n"
        "\n"
        "Displays information about the encrypted file, including format version,\n"
        "encryption algorithm, KDF, compression, and sizes of various components.\n"
    );
    
    cmd->callback([this]() { execute(); });
}

int InfoCommand::execute() {
    try {
        utils::Console::header("File Information");
        
        auto info = parse_file(input_file_);
        display_info(info);
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Failed to read file info: {}", e.what()));
        return 1;
    }
}

InfoCommand::FileInfo InfoCommand::parse_file(const std::string& path) {
    FileInfo info;
    
    // Open file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    
    info.file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Check if this is an enhanced format file
    if (!core::FileFormatHandler::is_legacy_format(path)) {
        // Enhanced format with header
        try {
            auto [header, ciphertext, auth_tag] = core::FileFormatHandler::read_file(path);
            
            info.has_header = true;
            info.version = fmt::format("{}.{}", header.version_major, header.version_minor);
            info.algorithm = engine_.algorithm_name(
                core::FileFormatHandler::from_algorithm_id(header.algorithm)
            );
            info.kdf = engine_.kdf_name(
                core::FileFormatHandler::from_kdf_id(header.kdf)
            );
            info.compression = core::FileFormatHandler::from_compression_id(header.compression);
            info.salt_size = header.salt.size();
            info.nonce_size = header.nonce.size();
            info.tag_size = auth_tag.size();
            info.data_size = ciphertext.size();
            info.header_size = header.size();
            info.compressed = header.compressed;
            
            return info;
        } catch (const std::exception& e) {
            throw std::runtime_error(fmt::format("Failed to parse enhanced format: {}", e.what()));
        }
    }
    
    // Legacy format: [salt (32 bytes)][nonce (12 bytes)][ciphertext + tag]
    if (info.file_size < 44) {
        throw std::runtime_error("File too small to be valid encrypted file");
    }
    
    info.has_header = false;
    info.salt_size = 32;
    info.nonce_size = 12;
    info.tag_size = 16;
    info.data_size = info.file_size - info.salt_size - info.nonce_size;
    
    return info;
}

void InfoCommand::display_info(const FileInfo& info) {
    fmt::print("\n");
    fmt::print("  📄 {:25} : {}\n", "File", input_file_);
    fmt::print("  📦 {:25} : {}\n", "Total Size", 
               utils::CryptoUtils::format_bytes(info.file_size));
    
    if (info.has_header) {
        fmt::print("  ✨ {:25} : {} (Enhanced Format)\n", "Format Version", info.version);
    } else {
        fmt::print("  ⚠️  {:25} : Legacy (No Header)\n", "Format Version");
    }
    
    fmt::print("\n");
    utils::Console::separator();
    fmt::print("\n");
    
    if (info.has_header) {
        fmt::print("  🔐 Algorithm Information:\n");
        fmt::print("     {:25} : {}\n", "Encryption", info.algorithm);
        fmt::print("     {:25} : {}\n", "Key Derivation", info.kdf);
        fmt::print("     {:25} : {}\n", "Compression", info.compression);
        fmt::print("     {:25} : {}\n", "Compressed", info.compressed ? "Yes" : "No");
        fmt::print("\n");
    }
    
    fmt::print("  🔒 Encryption Details:\n");
    if (info.has_header) {
        fmt::print("     {:25} : {} bytes\n", "Header Size", info.header_size);
    }
    fmt::print("     {:25} : {} bytes\n", "Salt", info.salt_size);
    fmt::print("     {:25} : {} bytes\n", "Nonce", info.nonce_size);
    fmt::print("     {:25} : {} bytes\n", "Auth Tag", info.tag_size);
    
    // For enhanced format, data_size is already just ciphertext (tag separate)
    // For legacy format, data_size includes tag, so subtract it
    size_t actual_data_size = info.has_header ? info.data_size : (info.data_size - info.tag_size);
    fmt::print("     {:25} : {}\n", "Encrypted Data", 
               utils::CryptoUtils::format_bytes(actual_data_size));
    
    if (verbose_) {
        fmt::print("\n");
        fmt::print("  📊 Statistics:\n");
        double overhead = static_cast<double>(info.salt_size + info.nonce_size + info.tag_size);
        if (info.has_header) {
            overhead += info.header_size;
        }
        double overhead_pct = (overhead / info.file_size) * 100.0;
        fmt::print("     {:25} : {:.2f}%\n", "Metadata Overhead", overhead_pct);
        
        if (info.has_header || verbose_) {
            size_t payload_size = actual_data_size;
            fmt::print("     {:25} : {:.2f}%\n", "Payload Ratio", 
                      (static_cast<double>(payload_size) / info.file_size) * 100.0);
        }
    }
    
    fmt::print("\n");
    
    if (!info.has_header) {
        utils::Console::warning("Note: This file uses legacy format. Re-encrypt with new version for enhanced metadata.");
    }
    
    fmt::print("\n");
}

} // namespace cli
} // namespace filevault
