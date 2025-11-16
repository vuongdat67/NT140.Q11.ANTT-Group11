#ifndef FILEVAULT_CLI_COMMANDS_BENCHMARK_CMD_HPP
#define FILEVAULT_CLI_COMMANDS_BENCHMARK_CMD_HPP

#include "filevault/cli/command.hpp"
#include "filevault/core/crypto_engine.hpp"

namespace filevault {
namespace cli {

class BenchmarkCommand : public ICommand {
public:
    explicit BenchmarkCommand(core::CryptoEngine& engine);
    
    std::string name() const override { return "benchmark"; }
    std::string description() const override { return "Benchmark algorithms"; }
    
    void setup(CLI::App& app) override;
    int execute() override;

private:
    core::CryptoEngine& engine_;
    std::string algorithm_;
    bool all_ = false;
};

} // namespace cli
} // namespace filevault

#endif
