#ifndef FILEVAULT_CLI_COMMANDS_COMPRESS_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_COMPRESS_CMD_HPP

#include <string>
#include "filevault/cli/command.hpp"

namespace filevault::cli {

/**
 * @brief Compress command - standalone compression/decompression
 * 
 * Compress or decompress files without encryption.
 * Supports: zlib, bzip2, lzma
 * 
 * Examples:
 *   filevault compress bigfile.log -a zlib -l 9 -o bigfile.zlib
 *   filevault compress --decompress bigfile.zlib -o bigfile.log
 */
class CompressCommand : public ICommand {
public:
    CompressCommand() = default;
    ~CompressCommand() override = default;

    std::string name() const override { return "compress"; }
    std::string description() const override { return "Compress or decompress files"; }
    void setup(CLI::App& app) override;
    int execute() override;
private:
    CLI::App* subcommand_ = nullptr;
    // Input/output files
    std::string input_file_;
    std::string output_file_;
    
    // Compression options
    std::string algorithm_ = "zlib";     // zlib, bzip2, lzma
    int level_ = 6;                       // 1-9
    bool decompress_ = false;             // Decompress mode
    bool verbose_ = false;
    bool benchmark_ = false;              // Show timing info
    bool auto_detect_ = false;            // Auto-detect algorithm
    
    // Helper methods
    int do_compress();
    int do_decompress();
    std::string detect_algorithm(const std::string& path);
    std::string generate_output_path(const std::string& input, bool compressing);
};

} // namespace filevault::cli

#endif // FILEVAULT_CLI_COMMANDS_COMPRESS_CMD_HPP
