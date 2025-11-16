# GitHub Copilot Instructions for FileVault

## 🎯 Project Overview

FileVault is a professional cross-platform file encryption CLI tool with comprehensive cryptographic algorithm support, built with modern C++20 and Botan 3.x.

**Core Philosophy:**
- Security first, performance second, usability third
- Clean code > clever code
- Test everything, trust nothing
- Document for your future self

---

## 📋 Project Standards

### Language & Compiler Requirements

```cpp
// REQUIRED: C++20 with Botan 3.x
#include <botan/version.h>
static_assert(BOTAN_VERSION_MAJOR >= 3, "Botan 3.x required");

// Use modern C++20 features
#include <concepts>
#include <ranges>
#include <span>
#include <format>  // C++20 formatting
```

### Coding Standards

#### 1. **File Organization**
```
Rule: ONE algorithm per file, ONE class per file
Good: src/algorithms/symmetric/aes.cpp
Bad:  src/crypto_stuff.cpp (multiple algorithms)

Rule: Header-only for templates, .cpp for implementations
Good: include/filevault/core/crypto_engine.hpp + src/core/crypto_engine.cpp
Bad:  everything in .hpp files
```

#### 2. **Naming Conventions**
```cpp
// Classes: PascalCase
class CryptoEngine { };

// Functions/methods: snake_case
void encrypt_file(const std::string& path);

// Constants: UPPER_SNAKE_CASE
constexpr size_t MAX_FILE_SIZE = 1024 * 1024 * 1024;

// Private members: trailing underscore
class MyClass {
private:
    int counter_;
    std::string name_;
};

// Namespaces: lowercase
namespace filevault::core::algorithms { }
```

#### 3. **Modern C++20 Patterns**

```cpp
// ✅ GOOD: Use concepts for type constraints
template<std::ranges::range R>
void process_data(R&& data) { }

// ✅ GOOD: Use std::span for contiguous data
void encrypt(std::span<const uint8_t> plaintext);

// ✅ GOOD: Use std::format (C++20)
auto msg = std::format("Encrypted {} bytes", size);

// ✅ GOOD: Use ranges
auto filtered = data | std::views::filter(is_valid)
                    | std::views::transform(encrypt);

// ❌ BAD: Old-style loops when ranges work
for (size_t i = 0; i < vec.size(); ++i) { }

// ✅ GOOD: Range-based for or ranges
for (const auto& item : vec) { }
```

#### 4. **Error Handling**

```cpp
// ✅ GOOD: Use std::expected (C++23) or custom Result type
[[nodiscard]] Result<std::vector<uint8_t>> encrypt_file(const std::string& path);

// ✅ GOOD: Specific exceptions with context
class EncryptionError : public std::runtime_error {
public:
    EncryptionError(const std::string& msg, std::string file, int line)
        : std::runtime_error(msg), file_(std::move(file)), line_(line) {}
private:
    std::string file_;
    int line_;
};

// ✅ GOOD: RAII for resource management
class SecureMemory {
public:
    SecureMemory(size_t size) : data_(new uint8_t[size]), size_(size) {
        lock_memory(data_, size_);
    }
    ~SecureMemory() {
        secure_zero(data_, size_);
        unlock_memory(data_, size_);
        delete[] data_;
    }
    // Delete copy, allow move
    SecureMemory(const SecureMemory&) = delete;
    SecureMemory& operator=(const SecureMemory&) = delete;
    SecureMemory(SecureMemory&&) noexcept = default;
    SecureMemory& operator=(SecureMemory&&) noexcept = default;
};

// ❌ BAD: Raw pointers without RAII
uint8_t* data = new uint8_t[size];
// ... (forgot to delete)
```

#### 5. **Security Guidelines**

```cpp
// ✅ CRITICAL: Generate unique salt/nonce for EVERY encryption
auto salt = generate_random_bytes(32);  // MUST be unique
auto nonce = generate_random_bytes(12); // MUST NEVER reuse

// ✅ CRITICAL: Clear sensitive data
void process_password(const std::string& password) {
    SecureVector<uint8_t> key = derive_key(password);
    // ... use key ...
    secure_zero(key.data(), key.size());  // MUST clear
}

// ✅ CRITICAL: Constant-time comparison
bool verify_tag(std::span<const uint8_t> a, std::span<const uint8_t> b) {
    return constant_time_compare(a.data(), b.data(), a.size());
}

// ❌ BAD: Timing attack vulnerable
if (computed_tag == provided_tag) { } // Variable time!

// ✅ GOOD: Store salt/nonce with ciphertext
struct EncryptedData {
    std::vector<uint8_t> salt;    // Store with data
    std::vector<uint8_t> nonce;   // Store with data
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> tag;     // GCM authentication tag
};
```

#### 6. **Botan 3.x Usage**

```cpp
// ✅ GOOD: Modern Botan 3.x API
#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/argon2.h>

// Initialize RNG
Botan::AutoSeeded_RNG rng;

// Create cipher
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Encryption);

// Key derivation with Argon2
auto key = Botan::argon2_generate(
    32,                          // output length
    password,                    // password
    salt.data(), salt.size(),    // salt
    65536,                       // memory in KB
    4,                           // parallelism
    100000                       // iterations
);

// ❌ BAD: Using deprecated Botan 2.x API
Botan::PBKDF* pbkdf = get_pbkdf("PBKDF2(SHA-256)"); // Old API
```

---

## 🔧 Development Workflow

### When Creating New Algorithms

```cpp
// Template for new algorithm implementation:

// 1. Create header: include/filevault/algorithms/[category]/[name].hpp
#ifndef FILEVAULT_ALGORITHMS_SYMMETRIC_CHACHA20_HPP
#define FILEVAULT_ALGORITHMS_SYMMETRIC_CHACHA20_HPP

#include "filevault/core/crypto_engine.hpp"
#include <botan/chacha20poly1305.h>

namespace filevault::algorithms::symmetric {

/**
 * @brief ChaCha20-Poly1305 AEAD cipher implementation
 * @see RFC 8439
 * @see https://botan.randombit.net/handbook/api_ref/cipher_modes.html
 */
class ChaCha20Poly1305 : public core::ICryptoAlgorithm {
public:
    std::string name() const override { return "ChaCha20-Poly1305"; }
    
    core::AlgorithmType type() const override {
        return core::AlgorithmType::CHACHA20_POLY1305;
    }
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config
    ) override;
    
    size_t key_size() const override { return 32; }  // 256 bits
    
    bool is_suitable_for(core::UserLevel level) const override {
        return level >= core::UserLevel::PROFESSIONAL;
    }

private:
    // Helper methods
    std::vector<uint8_t> do_cipher(
        std::span<const uint8_t> input,
        std::span<const uint8_t> key,
        std::span<const uint8_t> nonce,
        Botan::Cipher_Dir direction
    );
};

} // namespace filevault::algorithms::symmetric

#endif

// 2. Create implementation: src/algorithms/symmetric/chacha20.cpp
#include "filevault/algorithms/symmetric/chacha20.hpp"
#include <botan/auto_rng.h>

namespace filevault::algorithms::symmetric {

core::CryptoResult ChaCha20Poly1305::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    try {
        // Validate inputs
        if (key.size() != key_size()) {
            return {
                .success = false,
                .error_message = std::format("Invalid key size: {} (expected {})", 
                                           key.size(), key_size())
            };
        }
        
        // Generate unique nonce for THIS encryption
        Botan::AutoSeeded_RNG rng;
        std::vector<uint8_t> nonce(12);  // 96-bit nonce for ChaCha20
        rng.randomize(nonce.data(), nonce.size());
        
        // Encrypt
        auto start = std::chrono::high_resolution_clock::now();
        auto ciphertext = do_cipher(plaintext, key, nonce, Botan::Cipher_Dir::Encryption);
        auto end = std::chrono::high_resolution_clock::now();
        
        // Package result with nonce
        std::vector<uint8_t> result;
        result.reserve(nonce.size() + ciphertext.size());
        result.insert(result.end(), nonce.begin(), nonce.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
        
        return {
            .success = true,
            .data = std::move(result),
            .algorithm_used = type(),
            .original_size = plaintext.size(),
            .final_size = result.size(),
            .processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count()
        };
        
    } catch (const Botan::Exception& e) {
        return {
            .success = false,
            .error_message = std::format("Botan error: {}", e.what())
        };
    } catch (const std::exception& e) {
        return {
            .success = false,
            .error_message = std::format("Encryption failed: {}", e.what())
        };
    }
}

} // namespace filevault::algorithms::symmetric

// 3. Create test: tests/algorithms/test_chacha20.cpp
#include <catch2/catch_test_macros.hpp>
#include "filevault/algorithms/symmetric/chacha20.hpp"

TEST_CASE("ChaCha20-Poly1305 basic encryption/decryption", "[chacha20]") {
    using namespace filevault;
    
    algorithms::symmetric::ChaCha20Poly1305 cipher;
    
    SECTION("Encrypt and decrypt") {
        std::vector<uint8_t> plaintext = {'H', 'e', 'l', 'l', 'o'};
        std::vector<uint8_t> key(32, 0xAB);  // Test key
        
        core::EncryptionConfig config;
        
        auto encrypted = cipher.encrypt(plaintext, key, config);
        REQUIRE(encrypted.success);
        REQUIRE(encrypted.data.size() > plaintext.size());  // Has nonce + tag
        
        auto decrypted = cipher.decrypt(encrypted.data, key, config);
        REQUIRE(decrypted.success);
        REQUIRE(decrypted.data == plaintext);
    }
    
    SECTION("Invalid key size") {
        std::vector<uint8_t> plaintext = {'T', 'e', 's', 't'};
        std::vector<uint8_t> wrong_key(16);  // Too short
        
        core::EncryptionConfig config;
        auto result = cipher.encrypt(plaintext, wrong_key, config);
        REQUIRE_FALSE(result.success);
    }
}

// 4. Add NIST test vectors: tests/algorithms/nist_vectors/chacha20_vectors.cpp
TEST_CASE("ChaCha20 NIST test vectors", "[chacha20][nist]") {
    // From RFC 8439
    const std::vector<uint8_t> key = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        // ... (32 bytes total)
    };
    
    const std::vector<uint8_t> nonce = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a,
        0x00, 0x00, 0x00, 0x00
    };
    
    const std::vector<uint8_t> plaintext = {
        // ... test data
    };
    
    const std::vector<uint8_t> expected_ciphertext = {
        // ... expected output from RFC
    };
    
    // Test encryption matches NIST vectors
    auto result = encrypt_with_nonce(plaintext, key, nonce);
    REQUIRE(result == expected_ciphertext);
}
```

---

## 🧪 Testing Requirements

### Every Algorithm MUST Have:

1. **Unit tests** (basic functionality)
2. **NIST test vectors** (correctness validation)
3. **Edge case tests** (empty input, large input, etc.)
4. **Security tests** (no nonce reuse, unique outputs, etc.)

```cpp
// Security test template
TEST_CASE("Security properties", "[security][algorithm_name]") {
    SECTION("Same input, different outputs (unique nonces)") {
        std::vector<uint8_t> plaintext = {'T', 'e', 's', 't'};
        std::vector<uint8_t> key(32, 0xAB);
        
        std::vector<std::vector<uint8_t>> ciphertexts;
        for (int i = 0; i < 100; ++i) {
            auto result = cipher.encrypt(plaintext, key, config);
            REQUIRE(result.success);
            ciphertexts.push_back(result.data);
        }
        
        // All ciphertexts MUST be different
        for (size_t i = 0; i < ciphertexts.size(); ++i) {
            for (size_t j = i + 1; j < ciphertexts.size(); ++j) {
                REQUIRE(ciphertexts[i] != ciphertexts[j]);
            }
        }
    }
    
    SECTION("No information leakage") {
        // Test that metadata doesn't leak info
    }
}
```

---

## 📚 Documentation Requirements

### Code Comments

```cpp
// ✅ GOOD: Doxygen style with details
/**
 * @brief Derive encryption key from password using Argon2id
 * 
 * Uses Argon2id (RFC 9106) with recommended parameters for password-based
 * key derivation. Provides protection against:
 * - Rainbow table attacks (via unique salt)
 * - GPU cracking (via memory-hard function)
 * - Side-channel attacks (via data-independent memory access)
 * 
 * @param password User-provided password (will be securely cleared)
 * @param salt Unique random salt (min 32 bytes, MUST be unique per file)
 * @param iterations Number of iterations (min 100000 for OWASP 2023)
 * @param memory_kb Memory usage in KB (min 65536 for high security)
 * @param parallelism Degree of parallelism (typically 4)
 * @param key_length Output key length in bytes (typically 32 for AES-256)
 * 
 * @return Derived key
 * @throws std::invalid_argument if parameters are invalid
 * 
 * @see https://www.rfc-editor.org/rfc/rfc9106.html
 * @see https://cheatsheetseries.owasp.org/cheatsheets/Password_Storage_Cheat_Sheet.html
 * 
 * @note Salt MUST be stored with ciphertext for decryption
 * @warning Never reuse salt across different encryptions
 */
SecureVector<uint8_t> derive_key_argon2(
    const SecureString& password,
    std::span<const uint8_t> salt,
    uint32_t iterations = 100000,
    uint32_t memory_kb = 65536,
    uint32_t parallelism = 4,
    size_t key_length = 32
);

// ❌ BAD: Vague or missing comments
// Derives a key
std::vector<uint8_t> derive_key(const std::string& pwd, const std::vector<uint8_t>& s);
```

### Reference Documentation

Every algorithm file should include references:

```cpp
/**
 * @file aes.cpp
 * @brief AES (Advanced Encryption Standard) implementation using Botan 3.x
 * 
 * References:
 * - FIPS 197: Advanced Encryption Standard
 *   https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197.pdf
 * 
 * - NIST SP 800-38D: GCM Mode
 *   https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38d.pdf
 * 
 * - Botan AES Documentation:
 *   https://botan.randombit.net/handbook/api_ref/cipher_modes.html
 * 
 * Test Vectors:
 * - NIST CAVP: https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program
 */
```

---

## 🐛 Debugging Guidelines

### When Things Go Wrong:

```cpp
// ✅ GOOD: Detailed error messages with context
if (file_size > MAX_FILE_SIZE) {
    throw std::runtime_error(
        std::format("File too large: {} bytes (max: {} bytes)\n"
                   "File: {}\n"
                   "Consider using streaming encryption for large files",
                   file_size, MAX_FILE_SIZE, file_path)
    );
}

// ✅ GOOD: Debug logging (use spdlog)
spdlog::debug("Encrypting file: path={}, size={}, algorithm={}", 
              file_path, file_size, algorithm_name);

// ✅ GOOD: Assertions for invariants
assert(salt.size() >= 32 && "Salt must be at least 32 bytes");
assert(nonce.size() == 12 && "GCM nonce must be exactly 12 bytes");

// ✅ GOOD: Hex dump for debugging
void print_hex_dump(std::span<const uint8_t> data, const std::string& label) {
    std::cout << std::format("{} ({} bytes):\n", label, data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::format("{:02x} ", data[i]);
        if ((i + 1) % 16 == 0) std::cout << '\n';
    }
    std::cout << '\n';
}
```

### Common Pitfalls to Avoid:

```cpp
// ❌ PITFALL 1: Nonce reuse
static std::vector<uint8_t> global_nonce(12);  // NEVER DO THIS!

// ✅ CORRECT: Generate new nonce every time
Botan::AutoSeeded_RNG rng;
std::vector<uint8_t> nonce(12);
rng.randomize(nonce.data(), nonce.size());

// ❌ PITFALL 2: Not clearing sensitive data
std::string password = get_password();
auto key = derive_key(password);
// password still in memory! ❌

// ✅ CORRECT: Use SecureString and clear
SecureString password = get_password();
auto key = derive_key(password);
secure_zero(password.data(), password.size());

// ❌ PITFALL 3: Timing attacks in comparison
bool verify(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    return a == b;  // Variable-time comparison!
}

// ✅ CORRECT: Constant-time comparison
bool verify(std::span<const uint8_t> a, std::span<const uint8_t> b) {
    return Botan::constant_time_compare(a.data(), b.data(), a.size());
}

// ❌ PITFALL 4: Integer overflow
size_t total_size = num_blocks * block_size;  // May overflow!

// ✅ CORRECT: Check for overflow
if (num_blocks > SIZE_MAX / block_size) {
    throw std::overflow_error("Size calculation would overflow");
}
size_t total_size = num_blocks * block_size;
```

---

## 🔍 Code Review Checklist

Before committing, verify:

- [ ] No hardcoded keys, passwords, or secrets
- [ ] All sensitive data is cleared after use
- [ ] Salt/nonce generation is unique per encryption
- [ ] Error messages don't leak sensitive information
- [ ] All public APIs have documentation
- [ ] Tests cover happy path and edge cases
- [ ] NIST test vectors pass (if applicable)
- [ ] No compiler warnings (-Wall -Wextra -Wpedantic)
- [ ] Valgrind clean (no memory leaks)
- [ ] clang-format applied
- [ ] Includes are minimal and necessary

---

## 🎯 Performance Considerations

```cpp
// ✅ GOOD: Reserve capacity
std::vector<uint8_t> result;
result.reserve(plaintext.size() + overhead);

// ✅ GOOD: Move instead of copy
return std::move(large_vector);

// ✅ GOOD: Use string_view for read-only strings
void process(std::string_view input);

// ✅ GOOD: Avoid unnecessary allocations
std::span<const uint8_t> get_data() const { return data_; }

// ❌ BAD: Return by value for large data
std::vector<uint8_t> get_data() const { return data_; }
```

---

## 📦 Build System Integration

### CMakeLists.txt Guidelines:

```cmake
# Always specify minimum versions
cmake_minimum_required(VERSION 3.20)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Use target-based approach
add_library(my_lib STATIC ${SOURCES})
target_include_directories(my_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(my_lib PUBLIC botan-3)
```

---

## 🚀 Quick Reference

### Botan 3.x Common Operations:

```cpp
// Random generation
Botan::AutoSeeded_RNG rng;
std::vector<uint8_t> random_data(32);
rng.randomize(random_data.data(), random_data.size());

// Hashing
auto hash = Botan::HashFunction::create("SHA-256");
hash->update(data);
auto result = hash->final();

// Key derivation
auto key = Botan::argon2_generate(32, password, salt, 65536, 4, 100000);

// Cipher
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Encryption);
cipher->set_key(key);
cipher->start(nonce);
cipher->finish(buffer);
```

---

## ✅ Summary for Copilot

When writing code:
1. **Security first**: Unique nonces, clear sensitive data, constant-time ops
2. **Modern C++20**: Use concepts, ranges, span, format
3. **Botan 3.x**: Always use latest API patterns
4. **One file, one purpose**: Clean separation
5. **Test everything**: Unit + NIST + Security tests
6. **Document thoroughly**: Doxygen + references
7. **Handle errors**: Specific exceptions with context
8. **No warnings**: Clean compile with -Wall -Wextra -Wpedantic -Werror

**Remember:** This is cryptographic code. Mistakes can be catastrophic. When in doubt, be explicit, verbose, and safe.