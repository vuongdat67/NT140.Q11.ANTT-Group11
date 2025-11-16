#ifndef FILEVAULT_CLI_COMMANDS_ENCRYPT_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_ENCRYPT_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/core/crypto_engine.hpp"

namespace filevault {
namespace cli {

/**
 * @brief Encrypt command
 */
class EncryptCommand : public ICommand {
public:
    explicit EncryptCommand(core::CryptoEngine& engine);
    
    std::string name() const override { return "encrypt"; }
    std::string description() const override { return "Encrypt a file"; }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    core::CryptoEngine& engine_;
    
    // Command options
    std::string input_file_;
    std::string output_file_;
    std::string password_;
    std::string algorithm_ = "aes-256-gcm";
    std::string security_level_ = "medium";
    std::string kdf_ = "argon2id";
    bool compress_ = false;
    bool verbose_ = false;
};

} // namespace cli
} // namespace filevault

#endif // FILEVAULT_CLI_COMMANDS_ENCRYPT_CMD_HPP
