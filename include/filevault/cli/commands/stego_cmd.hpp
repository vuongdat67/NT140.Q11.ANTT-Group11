#ifndef FILEVAULT_CLI_COMMANDS_STEGO_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_STEGO_CMD_HPP

#include "filevault/cli/command.hpp"
#include <string>

namespace filevault::cli::commands {

/**
 * @brief Steganography command for embedding/extracting data in images
 * 
 * Supports LSB (Least Significant Bit) steganography for hiding data
 * within image files without visible changes.
 * 
 * Features:
 * - Embed secret data into PNG/BMP images
 * - Extract hidden data from stego images
 * - Calculate embedding capacity
 * - Configurable bits per channel (1-4)
 */
class StegoCommand : public ICommand {
public:
    std::string name() const override { return "stego"; }
    std::string description() const override {
        return "Hide or extract data from images using steganography";
    }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    // Subcommand handlers
    int do_embed();
    int do_extract();
    int do_capacity();
    
    // Options
    std::string operation_;           // "embed", "extract", or "capacity"
    std::string input_file_;          // Secret file (for embed) or stego image (for extract)
    std::string cover_image_;         // Cover image (for embed)
    std::string output_file_;         // Output path
    int bits_per_channel_ = 1;        // 1-4 bits per channel
    bool verbose_ = false;
};

} // namespace filevault::cli::commands

#endif // FILEVAULT_CLI_COMMANDS_STEGO_CMD_HPP
