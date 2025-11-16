#ifndef FILEVAULT_CLI_COMMAND_HPP
#define FILEVAULT_CLI_COMMAND_HPP

#include <string>
#include <CLI/CLI.hpp>

namespace filevault {
namespace cli {

/**
 * @brief Base interface for CLI commands
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    
    /**
     * @brief Get command name
     */
    virtual std::string name() const = 0;
    
    /**
     * @brief Get command description
     */
    virtual std::string description() const = 0;
    
    /**
     * @brief Setup CLI11 subcommand
     */
    virtual void setup(CLI::App& app) = 0;
    
    /**
     * @brief Execute the command
     */
    virtual int execute() = 0;
};

} // namespace cli
} // namespace filevault

#endif // FILEVAULT_CLI_COMMAND_HPP
