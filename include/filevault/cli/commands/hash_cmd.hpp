#ifndef FILEVAULT_CLI_COMMANDS_HASH_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_HASH_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/core/crypto_engine.hpp"

namespace filevault {
namespace cli {

class HashCommand : public ICommand {
public:
    explicit HashCommand(core::CryptoEngine& engine);
    
    std::string name() const override { return "hash"; }
    std::string description() const override { return "Calculate file hash"; }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    core::CryptoEngine& engine_;
    std::string input_file_;
    std::string algorithm_ = "sha256";
};

} // namespace cli
} // namespace filevault

#endif
