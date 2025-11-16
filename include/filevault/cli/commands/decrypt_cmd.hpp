#ifndef FILEVAULT_CLI_COMMANDS_DECRYPT_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_DECRYPT_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/core/crypto_engine.hpp"

namespace filevault {
namespace cli {

class DecryptCommand : public ICommand {
public:
    explicit DecryptCommand(core::CryptoEngine& engine);
    
    std::string name() const override { return "decrypt"; }
    std::string description() const override { return "Decrypt a file"; }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    core::CryptoEngine& engine_;
    std::string input_file_;
    std::string output_file_;
    std::string password_;
    bool verbose_ = false;
};

} // namespace cli
} // namespace filevault

#endif
