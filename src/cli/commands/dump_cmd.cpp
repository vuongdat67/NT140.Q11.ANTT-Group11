#include "filevault/cli/commands/dump_cmd.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/file_io.hpp"
#include <botan/hex.h>
#include <botan/base64.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <bitset>

namespace filevault {
namespace cli {
namespace commands {

DumpCommand::DumpCommand() = default;

void DumpCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name_, description_);
    
    cmd->add_option("file", file_path_, "File to dump")
        ->required()
        ->check(CLI::ExistingFile);
    
    cmd->add_option("-f,--format", format_, "Output format: hex, binary, base64")
        ->default_val("hex")
        ->check(CLI::IsMember({"hex", "binary", "base64"}));
    
    cmd->add_option("-n,--max-bytes", max_bytes_, 
                    "Maximum bytes to display (0 = all)")
        ->default_val(0);
    
    cmd->add_flag("--no-offset", [this](int64_t) { show_offset_ = false; },
                  "Hide byte offset column");
    
    cmd->add_flag("--no-ascii", [this](int64_t) { show_ascii_ = false; },
                  "Hide ASCII column (hex format only)");
    
    cmd->footer(
        "\nExamples:\n"
        "  Hex dump entire file:       filevault dump secret.bin\n"
        "  Binary dump first 256 bytes: filevault dump data.dat -f binary -n 256\n"
        "  Base64 dump file:           filevault dump image.png -f base64\n"
        "  Hex dump without ASCII:     filevault dump document.pdf --no-ascii\n"
        "\n"
        "Formats: hex (default), binary, base64\n"
    );
    
    cmd->callback([this]() { 
        int exit_code = this->execute();
        if (exit_code != 0) {
            throw CLI::RuntimeError(exit_code);
        }
    });
}

int DumpCommand::execute() {
    try {
        // Read file
        std::ifstream file(file_path_, std::ios::binary);
        if (!file) {
            utils::Console::error(fmt::format("Failed to open file: {}", file_path_));
            return 1;
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Determine how many bytes to read
        size_t bytes_to_read = max_bytes_ > 0 ? std::min(max_bytes_, file_size) : file_size;
        
        // Read file content
        std::vector<uint8_t> buffer(bytes_to_read);
        file.read(reinterpret_cast<char*>(buffer.data()), bytes_to_read);
        file.close();
        
        // Display header
        utils::Console::info(fmt::format("File: {}", file_path_));
        utils::Console::info(fmt::format("Size: {} bytes", file_size));
        if (max_bytes_ > 0 && bytes_to_read < file_size) {
            utils::Console::info(fmt::format("Showing first {} bytes", bytes_to_read));
        }
        utils::Console::info(fmt::format("Format: {}", format_));
        fmt::print("\n");
        
        // Dump content based on format
        if (format_ == "hex") {
            // Hexdump format: offset | hex bytes | ASCII
            for (size_t i = 0; i < buffer.size(); i += 16) {
                // Offset
                if (show_offset_) {
                    fmt::print("{:08x}  ", i);
                }
                
                // Hex bytes
                size_t line_len = std::min(size_t(16), buffer.size() - i);
                for (size_t j = 0; j < 16; j++) {
                    if (j < line_len) {
                        fmt::print("{:02x} ", buffer[i + j]);
                    } else {
                        fmt::print("   ");
                    }
                    if (j == 7) fmt::print(" ");
                }
                
                // ASCII
                if (show_ascii_) {
                    fmt::print(" |");
                    for (size_t j = 0; j < line_len; j++) {
                        uint8_t c = buffer[i + j];
                        fmt::print("{}", (c >= 32 && c < 127) ? char(c) : '.');
                    }
                    fmt::print("|");
                }
                
                fmt::print("\n");
            }
        } else if (format_ == "binary") {
            // Binary format: offset | binary representation
            for (size_t i = 0; i < buffer.size(); i += 8) {
                // Offset
                if (show_offset_) {
                    fmt::print("{:08x}  ", i);
                }
                
                // Binary bytes
                size_t line_len = std::min(size_t(8), buffer.size() - i);
                for (size_t j = 0; j < line_len; j++) {
                    fmt::print("{:08b} ", std::bitset<8>(buffer[i + j]).to_ulong());
                }
                
                fmt::print("\n");
            }
        } else if (format_ == "base64") {
            // Base64 format
            std::string base64 = Botan::base64_encode(buffer.data(), buffer.size());
            
            // Wrap at 76 characters (MIME standard)
            for (size_t i = 0; i < base64.length(); i += 76) {
                fmt::print("{}\n", base64.substr(i, 76));
            }
        }
        
        fmt::print("\n");
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Dump failed: {}", e.what()));
        return 1;
    }
}

} // namespace commands
} // namespace cli
} // namespace filevault
