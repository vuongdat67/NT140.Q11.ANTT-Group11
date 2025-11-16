#include "filevault/cli/commands/list_cmd.hpp"
#include "filevault/utils/console.hpp"
#include "filevault/utils/table_formatter.hpp"
#include <fmt/core.h>
#include <tabulate/table.hpp>

namespace filevault {
namespace cli {

ListCommand::ListCommand(core::CryptoEngine& engine)
    : engine_(engine) {
}

void ListCommand::setup(CLI::App& app) {
    auto* cmd = app.add_subcommand(name(), description());
    cmd->callback([this]() { execute(); });
}

int ListCommand::execute() {
    using namespace tabulate;
    
    utils::Console::header("FileVault - Available Algorithms");
    
    try {
        // Symmetric Encryption Table
        fmt::print("\n");
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::cyan), 
                  "Symmetric Encryption Algorithms\n");
        fmt::print("{}\n", std::string(80, '-'));
        
        Table sym_table;
        sym_table.add_row({"Algorithm", "Key Size", "Security", "Speed", "Notes"});
        sym_table.add_row({"AES-128-GCM", "128-bit", "Good", "***", "Fast, modern AEAD"});
        sym_table.add_row({"AES-192-GCM", "192-bit", "Strong", "**", "Balanced"});
        sym_table.add_row({"AES-256-GCM", "256-bit", "Maximum", "**", "Recommended"});
        sym_table.add_row({"ChaCha20-Poly1305", "256-bit", "Maximum", "***", "SW-optimized"});
        
        sym_table.format()
            .border_top("-")
            .border_bottom("-")
            .border_left("|")
            .border_right("|")
            .corner("+");
        
        sym_table[0].format()
            .font_align(FontAlign::center);
        
        std::cout << sym_table << "\n\n";
        
        // Classical Ciphers (Educational)
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::yellow), 
                  "Classical Ciphers (Educational Only - NOT SECURE)\n");
        fmt::print("{}\n", std::string(80, '-'));
        
        Table classical_table;
        classical_table.add_row({"Cipher", "Type", "Security", "Attack", "Purpose"});
        classical_table.add_row({"Caesar", "Shift", "BROKEN", "Brute-force", "Demo only (26 keys)"});
        classical_table.add_row({"Vigenere", "Polyalphabetic", "BROKEN", "Kasiski", "Show poly-cipher"});
        classical_table.add_row({"Playfair", "Digraph", "BROKEN", "Frequency", "Digraph example"});
        
        classical_table.format()
            .border_top("-")
            .border_bottom("-")
            .border_left("|")
            .border_right("|")
            .corner("+");
        
        classical_table[0].format()
            .font_align(FontAlign::center);
        
        std::cout << classical_table << "\n\n";
        
        // KDF Table
        fmt::print("Key Derivation Functions\n");
        fmt::print("{}\n", std::string(80, '-'));
    
    Table kdf_table;
    kdf_table.add_row({"KDF", "Type", "Resistance", "Speed", "Notes"});
        kdf_table.add_row({"Argon2id", "Memory-hard", "GPU/ASIC", "Slow", "Recommended"});
        kdf_table.add_row({"Argon2i", "Memory-hard", "Side-channel", "Slow", "Cache-safe"});
        kdf_table.add_row({"PBKDF2-SHA256", "Standard", "Basic", "Fast", "Fast, standard"});
        kdf_table.add_row({"PBKDF2-SHA512", "Standard", "Basic", "Fast", "Stronger"});
        kdf_table.add_row({"scrypt", "Memory-hard", "GPU/ASIC", "Slow", "Legacy"});
        
        kdf_table.format()
            .border_top("-")
            .border_bottom("-")
            .border_left("|")
            .border_right("|")
            .corner("+");
        
        kdf_table[0].format().font_align(FontAlign::center);
    
        std::cout << kdf_table << "\n\n";
        
        // Hash Functions Table
        fmt::print("Hash Functions\n");
        fmt::print("{}\n", std::string(80, '-'));
    
    Table hash_table;
    hash_table.add_row({"Algorithm", "Output", "Security", "Speed", "Notes"});
        hash_table.add_row({"SHA-256", "256-bit", "Strong", "**", "Standard"});
        hash_table.add_row({"SHA-512", "512-bit", "Maximum", "*", "Stronger"});
        hash_table.add_row({"BLAKE2b", "512-bit", "Maximum", "***", "Modern, fastest"});
        
        hash_table.format()
            .border_top("-")
            .border_bottom("-")
            .border_left("|")
            .border_right("|")
            .corner("+");
        
        hash_table[0].format().font_align(FontAlign::center);
    
        std::cout << hash_table << "\n\n";
        
        // Security Levels Table
        fmt::print("Security Levels\n");
        fmt::print("{}\n", std::string(80, '-'));
    
    Table sec_table;
    sec_table.add_row({"Level", "Iterations", "Memory", "Time", "Use Case"});
        sec_table.add_row({"weak", "2", "Low", "~5ms", "Testing only"});
        sec_table.add_row({"medium", "3", "Medium", "~10ms", "Recommended"});
        sec_table.add_row({"strong", "4", "High", "~20ms", "Sensitive data"});
        sec_table.add_row({"paranoid", "6", "Maximum", "~50ms", "Top secret"});
        
        sec_table.format()
            .border_top("-")
            .border_bottom("-")
            .border_left("|")
            .border_right("|")
            .corner("+");
        
        sec_table[0].format().font_align(FontAlign::center);
        
        std::cout << sec_table << "\n\n";
        
        // Tips
        fmt::print("\nTip: Use 'filevault benchmark' to measure performance\n\n");
        
    } catch (const std::exception& e) {
        // Fallback to simple list
        fmt::print("\n=== Symmetric Encryption ===\n");
        fmt::print("  - AES-128-GCM, AES-192-GCM, AES-256-GCM\n");
        fmt::print("  - ChaCha20-Poly1305\n\n");
        fmt::print("=== KDF ===\n");
        fmt::print("  - Argon2id (Recommended), Argon2i\n");
        fmt::print("  - PBKDF2-SHA256, PBKDF2-SHA512, scrypt\n\n");
    }
    
    return 0;
}

} // namespace cli
} // namespace filevault
