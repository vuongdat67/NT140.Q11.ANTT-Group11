#ifndef FILEVAULT_CLI_COMMANDS_CONFIG_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_CONFIG_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/utils/config.hpp"

namespace filevault {
namespace cli {

/**
 * @brief Config command - manage FileVault configuration
 */
class ConfigCommand : public ICommand {
public:
    ConfigCommand() = default;
    
    std::string name() const override { return "config"; }
    std::string description() const override { return "Manage FileVault configuration"; }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    int show_config();
    int set_value();
    int reset_config();
    
    std::string subcommand_;
    std::string key_;
    std::string value_;
};

} // namespace cli
} // namespace filevault

#endif // FILEVAULT_CLI_COMMANDS_CONFIG_CMD_HPP
