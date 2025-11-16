#ifndef FILEVAULT_CLI_COMMANDS_LIST_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_LIST_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/core/crypto_engine.hpp"

namespace filevault {
namespace cli {

class ListCommand : public ICommand {
public:
    explicit ListCommand(core::CryptoEngine& engine);
    
    std::string name() const override { return "list"; }
    std::string description() const override { return "List available algorithms"; }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    core::CryptoEngine& engine_;
};

} // namespace cli
} // namespace filevault

#endif
