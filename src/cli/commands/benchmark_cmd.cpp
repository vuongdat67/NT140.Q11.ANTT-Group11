#include "filevault/cli/commands/benchmark_cmd.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/crypto_utils.hpp"
#include <chrono>
#include <iomanip>

namespace filevault {
namespace cli {

BenchmarkCommand::BenchmarkCommand(core::CryptoEngine& engine)
    : engine_(engine) {
}

void BenchmarkCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name(), description());
    
    cmd->add_option("-a,--algorithm", algorithm_, "Algorithm to benchmark");
    cmd->add_flag("--all", all_, "Benchmark all algorithms");
    
    cmd->callback([this]() { execute(); });
}

int BenchmarkCommand::execute() {
    try {
        utils::Console::header("FileVault Benchmarks");
        
        // Test data sizes
        std::vector<size_t> sizes = {1024, 10240, 102400, 1048576}; // 1KB, 10KB, 100KB, 1MB
        
        fmt::print("\n📊 Encryption Performance:\n\n");
        fmt::print("{:<12} {:<15} {:<15} {:<15}\n", "Size", "AES-128-GCM", "AES-192-GCM", "AES-256-GCM");
        fmt::print("{:-<60}\n", "");
        
        for (auto size : sizes) {
            // Generate test data
            std::vector<uint8_t> plaintext(size, 0x42);
            std::vector<uint8_t> key(32, 0x00); // Dummy key
            
            fmt::print("{:<12}", utils::CryptoUtils::format_bytes(size));
            
            // Test each algorithm
            for (auto algo_type : {core::AlgorithmType::AES_128_GCM, 
                                   core::AlgorithmType::AES_192_GCM,
                                   core::AlgorithmType::AES_256_GCM}) {
                
                auto* algo = engine_.get_algorithm(algo_type);
                if (!algo) {
                    fmt::print("{:<15}", "N/A");
                    continue;
                }
                
                // Setup config
                core::EncryptionConfig config;
                config.nonce = engine_.generate_nonce(12);
                
                // Warm-up
                algo->encrypt(plaintext, key, config);
                
                // Benchmark (3 runs)
                double total_time = 0.0;
                for (int i = 0; i < 3; ++i) {
                    config.nonce = engine_.generate_nonce(12);
                    auto start = std::chrono::high_resolution_clock::now();
                    auto result = algo->encrypt(plaintext, key, config);
                    auto end = std::chrono::high_resolution_clock::now();
                    
                    if (!result.success) continue;
                    total_time += std::chrono::duration<double, std::milli>(end - start).count();
                }
                
                double avg_time = total_time / 3.0;
                double throughput = (size / 1024.0 / 1024.0) / (avg_time / 1000.0); // MB/s
                
                fmt::print("{:<15}", fmt::format("{:.2f} MB/s", throughput));
            }
            fmt::print("\n");
        }
        
        fmt::print("\n⚡ KDF Performance (256-bit key):\n\n");
        fmt::print("{:<15} {:<15} {:<15}\n", "Algorithm", "Time", "Rate");
        fmt::print("{:-<45}\n", "");
        
        // Benchmark KDF
        std::string password = "test_password_123";
        auto salt = engine_.generate_salt(32);
        
        for (auto kdf : {core::KDFType::ARGON2ID, core::KDFType::PBKDF2_SHA256, core::KDFType::SCRYPT}) {
            core::EncryptionConfig config;
            config.kdf = kdf;
            config.level = core::SecurityLevel::WEAK; // Use WEAK for benchmark speed
            config.apply_security_level();
            
            auto start = std::chrono::high_resolution_clock::now();
            auto key = engine_.derive_key(password, salt, config);
            auto end = std::chrono::high_resolution_clock::now();
            
            double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
            double rate = 1000.0 / time_ms; // keys/sec
            
            fmt::print("{:<15} {:<15} {:<15}\n", 
                      engine_.kdf_name(kdf),
                      fmt::format("{:.2f} ms", time_ms),
                      fmt::format("{:.1f} /sec", rate));
        }
        
        fmt::print("\n");
        return 0;
        
    } catch (const std::exception& e) {
        utils::Console::error(fmt::format("Benchmark failed: {}", e.what()));
        return 1;
    }
}

} // namespace cli
} // namespace filevault
