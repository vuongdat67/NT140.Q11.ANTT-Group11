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

namespace {

tabulate::Table create_styled_table(const std::vector<std::string>& headers) {
    tabulate::Table table;
    tabulate::Table::Row_t header_row;
    for (const auto& h : headers) {
        header_row.push_back(h);
    }
    table.add_row(header_row);
    
    // Style header
    table[0].format()
        .font_color(tabulate::Color::cyan)
        .font_align(tabulate::FontAlign::center)
        .font_style({tabulate::FontStyle::bold});
    
    return table;
}

void print_section_title(const std::string& title, const std::string& emoji = "") {
    fmt::print("\n");
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::cyan), 
              "{} {}\n", emoji, title);
}

} // anonymous namespace

int ListCommand::execute() {
    utils::Console::header("FileVault - Available Algorithms");
    
    // ==================== AEAD Symmetric ====================
    print_section_title("Symmetric Encryption (AEAD - Authenticated)", "🔐");
    {
        auto table = create_styled_table({"Algorithm", "Key Size", "Security", "Speed", "Notes"});
        table.add_row({"AES-128-GCM", "128-bit", "Good", "****", "Fast, NIST standard"});
        table.add_row({"AES-192-GCM", "192-bit", "Strong", "***", "Balanced"});
        table.add_row({"AES-256-GCM", "256-bit", "Maximum", "***", "Recommended"});
        table.add_row({"ChaCha20-Poly1305", "256-bit", "Maximum", "****", "SW-optimized"});
        table.add_row({"Serpent-256-GCM", "256-bit", "Maximum", "**", "AES finalist"});
        table.add_row({"Twofish-128-GCM", "128-bit", "Good", "***", "AES finalist"});
        table.add_row({"Twofish-256-GCM", "256-bit", "Maximum", "***", "AES finalist"});
        table.add_row({"Camellia-256-GCM", "256-bit", "Maximum", "***", "Japan (CRYPTREC)"});
        table.add_row({"ARIA-256-GCM", "256-bit", "Maximum", "***", "Korea (KS X 1213)"});
        table.add_row({"SM4-GCM", "128-bit", "Strong", "***", "China (GB/T 32907)"});
        
        // Highlight recommended row
        table[3].format().font_color(tabulate::Color::green);
        
        std::cout << table << std::endl;
    }
    
    // ==================== Non-AEAD Modes ====================
    print_section_title("Symmetric Block Modes (Non-AEAD - No Authentication)", "🔓");
    {
        auto table = create_styled_table({"Algorithm", "Key Size", "IV Size", "Mode", "Notes"});
        table.add_row({"AES-128-CBC", "128-bit", "16 bytes", "Block", "Requires HMAC"});
        table.add_row({"AES-256-CBC", "256-bit", "16 bytes", "Block", "Requires HMAC"});
        table.add_row({"AES-128-CTR", "128-bit", "16 bytes", "Stream", "Counter mode"});
        table.add_row({"AES-256-CTR", "256-bit", "16 bytes", "Stream", "Counter mode"});
        table.add_row({"AES-128-CFB", "128-bit", "16 bytes", "Stream", "Self-sync"});
        table.add_row({"AES-256-CFB", "256-bit", "16 bytes", "Stream", "Self-sync"});
        table.add_row({"AES-128-OFB", "128-bit", "16 bytes", "Stream", "Pre-computed"});
        table.add_row({"AES-256-OFB", "256-bit", "16 bytes", "Stream", "Pre-computed"});
        table.add_row({"AES-128-XTS", "256-bit", "16 bytes", "Disk", "Storage encryption"});
        table.add_row({"AES-256-XTS", "512-bit", "16 bytes", "Disk", "Storage encryption"});
        table.add_row({"3DES", "168-bit", "8 bytes", "Block", "Legacy only!"});
        
        std::cout << table << std::endl;
    }
    
    // ==================== Insecure Modes ====================
    fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "\n⚠️  INSECURE Modes (Testing Only):\n");
    {
        auto table = create_styled_table({"Algorithm", "Key Size", "IV Size", "Mode", "Warning"});
        table.add_row({"AES-128-ECB", "128-bit", "None", "Block", "NEVER USE!"});
        table.add_row({"AES-256-ECB", "256-bit", "None", "Block", "NEVER USE!"});
        
        // Red warning style
        for (size_t i = 1; i <= 2; ++i) {
            table[i].format().font_color(tabulate::Color::red);
        }
        
        std::cout << table << std::endl;
    }
    
    // ==================== Asymmetric ====================
    print_section_title("Asymmetric Encryption (Public-Key Cryptography)", "🔑");
    {
        auto table = create_styled_table({"Algorithm", "Key Size", "Security", "Speed", "Use Case"});
        table.add_row({"RSA-2048", "2048-bit", "Good", "*", "Key exchange"});
        table.add_row({"RSA-3072", "3072-bit", "Strong", "*", "Recommended minimum"});
        table.add_row({"RSA-4096", "4096-bit", "Maximum", "*", "High security"});
        table.add_row({"ECC-P256", "256-bit", "Strong", "***", "ECDH + AES-GCM hybrid"});
        table.add_row({"ECC-P384", "384-bit", "Strong", "**", "192-bit security"});
        table.add_row({"ECC-P521", "521-bit", "Maximum", "**", "256-bit security"});
        
        std::cout << table << std::endl;
    }
    
    // ==================== Post-Quantum ====================
    print_section_title("Post-Quantum Cryptography (NIST Selected)", "🛡️ ");
    {
        auto table = create_styled_table({"Algorithm", "Type", "NIST Level", "Speed", "Notes"});
        table.add_row({"Kyber-512", "KEM (ML-KEM)", "Level 1", "****", "Fast, 128-bit security"});
        table.add_row({"Kyber-768", "KEM (ML-KEM)", "Level 3", "***", "Balanced, 192-bit"});
        table.add_row({"Kyber-1024", "KEM (ML-KEM)", "Level 5", "***", "Maximum, 256-bit"});
        table.add_row({"Kyber-Hybrid", "KEM + AES-GCM", "Defense", "***", "Classical + PQC"});
        table.add_row({"Dilithium-2", "Signature", "Level 2", "****", "Fast signing"});
        table.add_row({"Dilithium-3", "Signature", "Level 3", "***", "Balanced"});
        table.add_row({"Dilithium-5", "Signature", "Level 5", "**", "Maximum security"});
        
        // Highlight recommended
        table[3].format().font_color(tabulate::Color::green);
        table[4].format().font_color(tabulate::Color::green);
        
        std::cout << table << std::endl;
    }
    
    // ==================== Classical Ciphers ====================
    print_section_title("Classical Ciphers (Educational Only - INSECURE)", "📚");
    {
        auto table = create_styled_table({"Cipher", "Type", "Attack Method", "Note"});
        table.add_row({"Caesar", "Shift", "Brute-force", "Only 26 keys"});
        table.add_row({"Vigenere", "Polyalphabetic", "Kasiski exam", "Repeated key weakness"});
        table.add_row({"Playfair", "Digraph", "Frequency", "600 digraphs"});
        table.add_row({"Hill", "Matrix", "Known-plaintext", "Linear algebra attack"});
        table.add_row({"Substitution", "Monoalphabetic", "Frequency", "26! permutations"});
        
        // Yellow warning style
        for (size_t i = 1; i <= 5; ++i) {
            table[i].format().font_color(tabulate::Color::yellow);
        }
        
        std::cout << table << std::endl;
    }
    
    // ==================== KDF ====================
    print_section_title("Key Derivation Functions", "🔐");
    {
        auto table = create_styled_table({"KDF", "Type", "Resistance", "Speed", "Note"});
        table.add_row({"Argon2id", "Memory-hard", "GPU/ASIC", "Slow", "Recommended"});
        table.add_row({"Argon2i", "Memory-hard", "Side-channel", "Slow", "Cache-safe"});
        table.add_row({"PBKDF2-SHA256", "Standard", "Basic", "Fast", "Legacy support"});
        table.add_row({"PBKDF2-SHA512", "Standard", "Basic", "Fast", "Stronger"});
        table.add_row({"scrypt", "Memory-hard", "GPU/ASIC", "Slow", "Legacy"});
        
        table[1].format().font_color(tabulate::Color::green);
        
        std::cout << table << std::endl;
    }
    
    // ==================== Hash Functions ====================
    print_section_title("Hash Functions", "🔒");
    {
        auto table = create_styled_table({"Algorithm", "Output", "Security", "Speed", "Note"});
        table.add_row({"MD5", "128-bit", "BROKEN", "****", "Legacy only!"});
        table.add_row({"SHA-1", "160-bit", "BROKEN", "***", "Legacy only!"});
        table.add_row({"SHA-256", "256-bit", "Strong", "**", "Standard"});
        table.add_row({"SHA-512", "512-bit", "Maximum", "*", "Stronger"});
        table.add_row({"SHA3-256", "256-bit", "Strong", "**", "NIST SHA-3"});
        table.add_row({"SHA3-512", "512-bit", "Maximum", "*", "NIST SHA-3"});
        table.add_row({"BLAKE2b", "512-bit", "Maximum", "***", "Modern, fastest"});
        
        // Mark insecure hashes in red
        table[1].format().font_color(tabulate::Color::red);
        table[2].format().font_color(tabulate::Color::red);
        // Mark recommended in green
        table[7].format().font_color(tabulate::Color::green);
        
        std::cout << table << std::endl;
    }
    
    // ==================== Security Levels ====================
    print_section_title("Security Levels", "🛡️ ");
    {
        auto table = create_styled_table({"Level", "Iterations", "Memory", "Time", "Use Case"});
        table.add_row({"weak", "1", "4 MB", "~2ms", "Testing only"});
        table.add_row({"medium", "2", "16 MB", "~10ms", "Recommended"});
        table.add_row({"strong", "3", "64 MB", "~30ms", "Sensitive data"});
        table.add_row({"paranoid", "4", "128 MB", "~60ms", "Top secret"});
        
        table[1].format().font_color(tabulate::Color::yellow);
        table[2].format().font_color(tabulate::Color::green);
        table[3].format().font_color(tabulate::Color::cyan);
        table[4].format().font_color(tabulate::Color::magenta);
        
        std::cout << table << std::endl;
    }
    
    // ==================== Usage Examples ====================
    print_section_title("Usage Examples", "💡");
    {
        auto table = create_styled_table({"Category", "Command"});
        table.add_row({"AEAD (Recommended)", "filevault encrypt input.txt -a aes-256-gcm -s medium"});
        table.add_row({"ChaCha20", "filevault encrypt data.zip -a chacha20-poly1305"});
        table.add_row({"PQC Keygen", "filevault keygen -a kyber-1024 -o quantum-key"});
        table.add_row({"PQC Encrypt", "filevault encrypt secret.txt -a kyber-1024-hybrid"});
        table.add_row({"RSA Keygen", "filevault keygen -a rsa-4096 -o mykey"});
        table.add_row({"RSA Encrypt", "filevault encrypt file.txt --pubkey mykey.pub"});
        table.add_row({"Block Mode", "filevault encrypt disk.img -a aes-256-xts"});
        table.add_row({"International", "filevault encrypt file.txt -a camellia-256-gcm"});
        
        // Style command column
        table.column(1).format().font_color(tabulate::Color::white);
        
        std::cout << table << std::endl;
    }
    
    fmt::print(fmt::emphasis::italic | fmt::fg(fmt::color::gray), 
              "\nFor more information: filevault --help or visit documentation\n\n");
    
    return 0;
}

} // namespace cli
} // namespace filevault
