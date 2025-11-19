#ifndef FILEVAULT_CLI_COMMANDS_ARCHIVE_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_ARCHIVE_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/core/crypto_engine.hpp"
#include <string>
#include <vector>

namespace filevault::cli::commands {

/**
 * @brief Archive command - compress and encrypt multiple files
 * 
 * Creates an archive from multiple files, compresses it, then encrypts.
 * Format: [metadata][file1_data][file2_data]...
 */
class ArchiveCommand : public cli::ICommand {
public:
    explicit ArchiveCommand(core::CryptoEngine& engine) 
        : engine_(engine) {}
    
    std::string name() const override { return "archive"; }
    
    std::string description() const override {
        return "Create encrypted archive from multiple files";
    }
    
    void setup(CLI::App& app) override;
    int execute() override;
    
private:
    int do_create();
    int do_extract();
    
    core::CryptoEngine& engine_;
    
    // Options
    std::vector<std::string> input_files_;
    std::string output_file_;
    std::string password_;
    std::string algorithm_ = "aes-256-gcm";
    std::string compression_ = "zlib";
    std::string kdf_ = "argon2id";
    std::string security_level_ = "medium";
    bool extract_ = false;
    bool verbose_ = false;
    std::string extract_dir_ = ".";
};

} // namespace filevault::cli::commands

#endif // FILEVAULT_CLI_COMMANDS_ARCHIVE_CMD_HPP
