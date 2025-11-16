#include "filevault/cli/commands/hash_cmd.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/file_io.hpp"
#include "filevault/utils/crypto_utils.hpp"
#include <botan/hash.h>
#include <chrono>

namespace filevault {
namespace cli {

HashCommand::HashCommand(core::CryptoEngine& engine)
    : engine_(engine) {
}

void HashCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name(), description());
    
    cmd->add_option("input", input_file_, "Input file")
        ->required()
        ->check(CLI::ExistingFile);
    
    cmd->add_option("-a,--algorithm", algorithm_, "Hash algorithm")
        ->check(CLI::IsMember({"sha256", "sha512", "blake2b"}));
    
    cmd->callback([this]() { execute(); });
}

int HashCommand::execute() {
    try {
        utils::Console::header("FileVault Hash");
        
        // Read file
        auto file_result = utils::FileIO::read_file(input_file_);
        if (!file_result) {
            utils::Console::error(file_result.error_message);
            return 1;
        }
        
        const auto& data = file_result.value;
        utils::Console::info(fmt::format("File: {} ({} bytes)", 
                           input_file_, 
                           utils::CryptoUtils::format_bytes(data.size())));
        utils::Console::separator();
        
        // Map algorithm names to Botan names
        std::string botan_algo;
        if (algorithm_ == "sha256") botan_algo = "SHA-256";
        else if (algorithm_ == "sha512") botan_algo = "SHA-512";
        else if (algorithm_ == "blake2b") botan_algo = "BLAKE2b";
        else {
            utils::Console::error(fmt::format("Unknown algorithm: {}", algorithm_));
            return 1;
        }
        
        // Create hash
        auto hash_fn = Botan::HashFunction::create(botan_algo);
        if (!hash_fn) {
            utils::Console::error(fmt::format("Failed to create {} hash", botan_algo));
            return 1;
        }
        
        // Compute hash
        auto start = std::chrono::high_resolution_clock::now();
        hash_fn->update(data.data(), data.size());
        auto hash_bytes = hash_fn->final();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        // Convert to hex
        auto hash_hex = utils::CryptoUtils::hex_encode(
            std::vector<uint8_t>(hash_bytes.begin(), hash_bytes.end())
        );
        
        // Display result
        utils::Console::success(fmt::format("{}: {}", algorithm_, hash_hex));
        utils::Console::info(fmt::format("Computed in {:.2f}ms", duration));
        
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Hash failed: {}", e.what()));
        return 1;
    }
}

} // namespace cli
} // namespace filevault
