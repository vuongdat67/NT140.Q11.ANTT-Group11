# FileVault Troubleshooting Guide

## 🔧 Common Issues & Solutions

### Build Issues

#### 1. "Botan 3.x not found"

**Problem:**
```
CMake Error: Could not find Botan 3.x
```

**Solution:**
```bash
# Install Botan 3.x from source
git clone https://github.com/randombit/botan.git
cd botan
./configure.py --prefix=/usr/local
make -j$(nproc)
sudo make install

# Update library path
echo '/usr/local/lib' | sudo tee /etc/ld.so.conf.d/botan.conf
sudo ldconfig

# Verify installation
pkg-config --modversion botan-3
```

**Alternative (vcpkg):**
```bash
vcpkg install botan:x64-linux
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake ..
```

---

#### 2. "C++20 not supported"

**Problem:**
```
error: This file requires compiler and library support for the ISO C++ 2020 standard
```

**Solution:**
```bash
# Check compiler version
g++ --version  # Need GCC 10+
clang++ --version  # Need Clang 12+

# Update CMakeLists.txt
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force C++20
cmake -DCMAKE_CXX_STANDARD=20 ..
```

---

#### 3. "undefined reference to botan_*"

**Problem:**
```
undefined reference to `Botan::AutoSeeded_RNG::AutoSeeded_RNG()'
```

**Solution:**
```cmake
# Make sure to link Botan 3.x
find_package(botan REQUIRED)
target_link_libraries(your_target PRIVATE botan::botan-3)

# NOT botan-2!
```

**Manual linking:**
```bash
g++ -std=c++20 main.cpp -lbotan-3 -o filevault
```

---

### Runtime Issues

#### 4. "Algorithm not available"

**Problem:**
```cpp
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", ...);
// cipher is nullptr
```

**Debug:**
```cpp
// List available algorithms
#include <botan/version.h>

auto modes = Botan::Cipher_Mode::providers("AES-256/GCM");
if (modes.empty()) {
    std::cerr << "AES-256/GCM not available\n";
    
    // Check what IS available
    for (const auto& algo : {"AES-128/GCM", "AES-256/CBC", "ChaCha20Poly1305"}) {
        auto test = Botan::Cipher_Mode::create(algo, Botan::Cipher_Dir::Encryption);
        std::cout << algo << ": " << (test ? "✓" : "✗") << "\n";
    }
}
```

**Solution:**
Check Botan build configuration:
```bash
# Rebuild Botan with all modules
./configure.py --minimized-build --enable-modules=aes,gcm,sha2,argon2
```

---

#### 5. "Invalid_IV_Length exception"

**Problem:**
```cpp
Botan::Invalid_IV_Length: AES-256/GCM requires exactly 12 byte nonce
```

**Solution:**
```cpp
// ❌ WRONG: Wrong nonce size
std::vector<uint8_t> nonce(16);  // Too big for GCM!

// ✅ CORRECT: Exact size
std::vector<uint8_t> nonce(12);  // 96 bits for GCM

// Different modes need different IV sizes:
// GCM: 12 bytes (96 bits)
// CBC: 16 bytes (128 bits) for AES
// ChaCha20: 8 or 12 bytes
```

---

#### 6. "Integrity_Failure on decrypt"

**Problem:**
```cpp
Botan::Integrity_Failure: GCM tag verification failed
```

**Causes:**
1. Wrong password/key
2. Corrupted ciphertext
3. Nonce not saved correctly
4. Tag not saved correctly

**Debug:**
```cpp
try {
    cipher->finish(buffer);
} catch (const Botan::Integrity_Failure& e) {
    std::cerr << "Authentication failed!\n";
    std::cerr << "Possible causes:\n";
    std::cerr << "- Wrong password\n";
    std::cerr << "- Corrupted file\n";
    std::cerr << "- Tampered data\n";
    
    // Check file format
    if (buffer.size() < 16) {
        std::cerr << "Buffer too small for GCM tag\n";
    }
}
```

---

#### 7. Segmentation Fault

**Problem:**
```
Segmentation fault (core dumped)
```

**Debug with gdb:**
```bash
# Compile with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with gdb
gdb ./filevault
(gdb) run encrypt test.txt
(gdb) bt  # backtrace when crash

# Or catch crash
(gdb) catch throw
(gdb) run
```

**Debug with Valgrind:**
```bash
valgrind --leak-check=full \
         --track-origins=yes \
         --show-leak-kinds=all \
         ./filevault encrypt test.txt
```

**Common causes:**
```cpp
// ❌ Dereferencing nullptr
auto cipher = Botan::Cipher_Mode::create("BadAlgo", ...);
cipher->set_key(key);  // CRASH if cipher is null!

// ✅ Check before use
if (!cipher) {
    throw std::runtime_error("Failed to create cipher");
}

// ❌ Buffer overflow
std::vector<uint8_t> buffer(10);
cipher->finish(buffer);  // May need more space!

// ✅ Reserve enough space
size_t min_size = cipher->minimum_final_size();
buffer.reserve(buffer.size() + min_size);
```

---

### Security Issues

#### 8. "Same ciphertext for same plaintext"

**Problem:**
```cpp
auto ct1 = encrypt("test", "password");
auto ct2 = encrypt("test", "password");
// ct1 == ct2  ← SECURITY BUG!
```

**Cause:** Reusing nonce/IV

**Solution:**
```cpp
// ✅ Generate new nonce EVERY encryption
CryptoResult encrypt(const std::vector<uint8_t>& plaintext,
                    const std::string& password) {
    Botan::AutoSeeded_RNG rng;
    
    // NEW nonce every call
    std::vector<uint8_t> nonce(12);
    rng.randomize(nonce.data(), nonce.size());
    
    // ... encrypt with this nonce
}
```

**Verify:**
```cpp
TEST_CASE("Unique ciphertexts") {
    for (int i = 0; i < 100; ++i) {
        auto ct = encrypt("same plaintext", "same password");
        all_ciphertexts.insert(ct);
    }
    
    // Must have 100 unique ciphertexts
    REQUIRE(all_ciphertexts.size() == 100);
}
```

---

#### 9. Memory not cleared

**Problem:**
Password/key still in memory after use

**Check:**
```bash
# Run with gdb and check memory
gdb ./filevault
(gdb) break after_encryption
(gdb) run
(gdb) x/32xb password_address  # Should be all zeros
```

**Solution:**
```cpp
// ✅ Use SecureVector
Botan::secure_vector<uint8_t> key(32);
// Automatically zeroed on destruction

// ✅ Manual clearing
void clear_sensitive_data(std::vector<uint8_t>& data) {
    #ifdef _WIN32
        SecureZeroMemory(data.data(), data.size());
    #else
        volatile uint8_t* p = data.data();
        for (size_t i = 0; i < data.size(); ++i) {
            p[i] = 0;
        }
    #endif
    data.clear();
}
```

---

### Performance Issues

#### 10. "Encryption is too slow"

**Benchmark:**
```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
// ... encryption code ...
auto end = std::chrono::high_resolution_clock::now();

auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Time: " << duration.count() << " ms\n";

// Calculate throughput
double mb_per_sec = (file_size_bytes / 1024.0 / 1024.0) / 
                    (duration.count() / 1000.0);
std::cout << "Throughput: " << mb_per_sec << " MB/s\n";
```

**Optimization tips:**
```cpp
// 1. Use hardware acceleration
if (Botan::CPUID::has_aes_ni()) {
    // AES-NI will be used automatically
    std::cout << "AES-NI available\n";
}

// 2. Choose fast algorithm
"ChaCha20Poly1305"  // ~800 MB/s (software)
"AES-256/GCM"       // ~500 MB/s (software), 2-4 GB/s (AES-NI)

// 3. Reduce KDF iterations (ONLY for testing!)
// Production: 100,000+
// Testing: 10,000

// 4. Process in chunks
const size_t CHUNK_SIZE = 1024 * 1024;  // 1MB
for (size_t offset = 0; offset < total; offset += CHUNK_SIZE) {
    process_chunk(offset, CHUNK_SIZE);
}

// 5. Parallel processing
#pragma omp parallel for
for (size_t i = 0; i < num_files; ++i) {
    encrypt_file(files[i]);
}
```

---

#### 11. "Out of memory"

**Problem:**
```
std::bad_alloc
terminate called after throwing an instance of 'std::bad_alloc'
```

**Solution:**
```cpp
// ❌ BAD: Load entire file to memory
std::vector<uint8_t> file_data = read_entire_file(path);  // 10GB file!

// ✅ GOOD: Stream processing
void encrypt_large_file(const std::string& input, 
                       const std::string& output) {
    std::ifstream in(input, std::ios::binary);
    std::ofstream out(output, std::ios::binary);
    
    const size_t CHUNK = 1024 * 1024;  // 1MB chunks
    std::vector<uint8_t> buffer(CHUNK);
    
    auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", ...);
    cipher->start(nonce);
    
    while (in.read(reinterpret_cast<char*>(buffer.data()), CHUNK)) {
        size_t bytes_read = in.gcount();
        Botan::secure_vector<uint8_t> chunk(buffer.begin(), 
                                            buffer.begin() + bytes_read);
        cipher->update(chunk);
        out.write(reinterpret_cast<const char*>(chunk.data()), 
                 chunk.size());
    }
    
    cipher->finish(buffer);
    out.write(reinterpret_cast<const char*>(buffer.data()), 
             buffer.size());
}
```

---

### File Format Issues

#### 12. "Can't decrypt own encrypted files"

**Problem:**
File encrypted with FileVault can't be decrypted

**Debug:**
```cpp
// Verify file format
void verify_file_format(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    
    // Check magic header
    char magic[8];
    file.read(magic, 8);
    
    if (std::memcmp(magic, "FVAULT01", 8) != 0) {
        std::cerr << "Invalid magic header: ";
        for (int i = 0; i < 8; ++i) {
            std::cerr << std::hex << (int)(uint8_t)magic[i] << " ";
        }
        std::cerr << "\n";
        return;
    }
    
    // Read version
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), 4);
    std::cout << "Version: " << version << "\n";
    
    // Read algorithm ID
    uint32_t algo_id;
    file.read(reinterpret_cast<char*>(&algo_id), 4);
    std::cout << "Algorithm: " << algo_id << "\n";
    
    // Continue parsing...
}
```

**Common mistakes:**
```cpp
// ❌ WRONG: Not saving salt
auto encrypted = encrypt(data, key, nonce);
write_file(output, encrypted);  // Lost salt and nonce!

// ✅ CORRECT: Save metadata
struct FileHeader {
    char magic[8] = {'F','V','A','U','L','T','0','1'};
    uint32_t version = 1;
    uint32_t algo_id;
    uint32_t salt_length;
    std::vector<uint8_t> salt;
    uint32_t nonce_length;
    std::vector<uint8_t> nonce;
    // ...
};

void write_encrypted_file(const std::string& path,
                         const FileHeader& header,
                         const std::vector<uint8_t>& ciphertext) {
    std::ofstream file(path, std::ios::binary);
    
    // Write header
    file.write(header.magic, 8);
    file.write(reinterpret_cast<const char*>(&header.version), 4);
    // ... write all fields
    
    // Write ciphertext
    file.write(reinterpret_cast<const char*>(ciphertext.data()),
              ciphertext.size());
}
```

---

### Testing Issues

#### 13. "NIST test vectors failing"

**Problem:**
```
Test failed: AES-256-GCM NIST vector
Expected: a1b2c3...
Got:      d4e5f6...
```

**Debug:**
```cpp
void debug_encryption(const std::vector<uint8_t>& plaintext,
                     const std::vector<uint8_t>& key,
                     const std::vector<uint8_t>& nonce) {
    std::cout << "=== Debug Encryption ===\n";
    
    std::cout << "Plaintext (" << plaintext.size() << " bytes):\n";
    print_hex(plaintext);
    
    std::cout << "Key (" << key.size() << " bytes):\n";
    print_hex(key);
    
    std::cout << "Nonce (" << nonce.size() << " bytes):\n";
    print_hex(nonce);
    
    auto cipher = Botan::Cipher_Mode::create("AES-256/GCM",
                                             Botan::Cipher_Dir::Encryption);
    cipher->set_key(key);
    
    Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
    cipher->start(nonce);
    cipher->finish(buffer);
    
    std::cout << "Ciphertext + Tag (" << buffer.size() << " bytes):\n";
    print_hex(buffer);
}

void print_hex(const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (int)data[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << "\n";
    }
    std::cout << std::dec << "\n";
}
```

**Common issues:**
1. Wrong byte order (endianness)
2. Not including authentication tag in comparison
3. Using wrong mode (CBC instead of GCM)
4. Including extra data in comparison

---

## 🔍 Debugging Tools

### 1. Verbose Logging

```cpp
// Add logging macro
#ifdef DEBUG
#define LOG_DEBUG(msg) std::cout << "[DEBUG] " << msg << "\n"
#else
#define LOG_DEBUG(msg)
#endif

// Usage
LOG_DEBUG("Derived key: " << hex_encode(key));
LOG_DEBUG("Nonce: " << hex_encode(nonce));
```

### 2. Memory Leak Detection

```bash
# Valgrind
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./filevault encrypt test.txt

# Expected output for clean code:
# All heap blocks were freed -- no leaks are possible
```

### 3. AddressSanitizer

```bash
# Compile with sanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make

# Run
./filevault encrypt test.txt

# Will catch:
# - Use after free
# - Buffer overflows
# - Memory leaks
```

### 4. UndefinedBehaviorSanitizer

```bash
cmake -DCMAKE_CXX_FLAGS="-fsanitize=undefined -g" ..
make

# Catches:
# - Integer overflow
# - Null pointer dereference
# - Invalid shifts
```

---

## 📞 Getting Help

### Before Asking

1. **Read error message carefully**
   - Note exact error text
   - Check line numbers
   - Look for stack trace

2. **Minimal reproducible example**
   ```cpp
   // Simplify to smallest code that shows problem
   #include <botan/cipher_mode.h>
   
   int main() {
       auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", ...);
       // ... minimal code that triggers issue
   }
   ```

3. **Gather information**
   ```bash
   # System info
   uname -a
   g++ --version
   cmake --version
   
   # Botan info
   pkg-config --modversion botan-3
   
   # Build info
   cat CMakeCache.txt | grep BOTAN
   ```

### Where to Ask

1. **FileVault Issues**: GitHub Issues
2. **Botan Questions**: https://github.com/randombit/botan/discussions
3. **C++ Questions**: Stack Overflow (tag: c++, cryptography)

### Bug Report Template

```markdown
**Environment:**
- OS: Ubuntu 22.04
- Compiler: GCC 11.3
- Botan version: 3.2.0
- FileVault commit: abc123

**Issue:**
Brief description

**Steps to Reproduce:**
1. Step 1
2. Step 2
3. Step 3

**Expected behavior:**
What should happen

**Actual behavior:**
What actually happens

**Code:**
```cpp
// Minimal example
```

**Error output:**
```
Paste full error message
```

**Additional context:**
Any other relevant information
```

---

## ✅ Prevention Checklist

Before committing code:

- [ ] Code compiles with no warnings (-Wall -Wextra -Wpedantic)
- [ ] All tests pass
- [ ] Valgrind shows no leaks
- [ ] AddressSanitizer shows no errors
- [ ] Code formatted (clang-format)
- [ ] Documentation updated
- [ ] NIST vectors pass (if applicable)
- [ ] Security review done
- [ ] Performance acceptable

---

## 🎯 Quick Fixes

| Problem | Quick Fix |
|---------|-----------|
| Botan not found | `export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig` |
| Wrong C++ version | `cmake -DCMAKE_CXX_STANDARD=20 ..` |
| Link error | Add `-lbotan-3` to link flags |
| Slow encryption | Check KDF iterations (100k max for testing) |
| Memory leak | Use `Botan::secure_vector` |
| Segfault | Check for nullptr before use |
| Auth failure | Verify nonce and tag are saved |
| Same ciphertext | Generate new nonce each time |

Remember: **Read the error message**, **check the docs**, **write a test**!