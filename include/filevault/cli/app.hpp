#ifndef FILEVAULT_CLI_APP_HPP
#define FILEVAULT_CLI_APP_HPP

#include <memory>
#include <vector>
#include <CLI/CLI.hpp>
#include "command.hpp"
#include "filevault/core/crypto_engine.hpp"

namespace filevault {
namespace cli {

/**
 * @brief Main CLI application
 */
class Application {
public:
    Application();
    ~Application();
    
    /**
     * @brief Initialize application and register commands
     */
    void initialize();
    
    /**
     * @brief Run application with command line arguments
     */
    int run(int argc, char** argv);

private:
    void register_commands();
    void setup_logging();
    
    CLI::App app_;
    std::unique_ptr<core::CryptoEngine> engine_;
    std::vector<std::unique_ptr<ICommand>> commands_;
    
    // Global options
    bool verbose_ = false;
    std::string log_level_ = "info";
};

} // namespace cli
} // namespace filevault

#endif // FILEVAULT_CLI_APP_HPP
