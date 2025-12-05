#ifndef FILEVAULT_CLI_COMMANDS_DUMP_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_DUMP_CMD_HPP

#include "../command.hpp"
#include <string>

namespace filevault {
namespace cli {
namespace commands {

/**
 * @brief Command to dump file content in various formats (hex, binary, base64)
 */
class DumpCommand : public ICommand {
public:
    DumpCommand();
    
    std::string name() const override { return name_; }
    std::string description() const override { return description_; }
    
    void setup(CLI::App& app) override;
    int execute() override;
    
private:
    std::string name_ = "dump";
    std::string description_ = "View file content in hex, binary, or base64 format";
    
    std::string file_path_;
    std::string format_ = "hex";  // Default format
    size_t max_bytes_ = 0;  // 0 = show all
    bool show_offset_ = true;
    bool show_ascii_ = true;
};

} // namespace commands
} // namespace cli
} // namespace filevault

#endif // FILEVAULT_CLI_COMMANDS_DUMP_CMD_HPP
