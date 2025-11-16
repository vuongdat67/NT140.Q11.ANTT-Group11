# FileVault Quick Reference

## 🚀 Quick Setup (Windows 11)

```powershell
# 1. Install prerequisites
winget install Microsoft.VisualStudio.2022.BuildTools
winget install Python.Python.3.12
winget install Kitware.CMake
pip install conan

# 2. Clone and build
git clone https://github.com/yourusername/filevault.git
cd filevault
mkdir build && cd build

# 3. Conan TẢI pre-compiled packages (5-10 phút lần đầu)
conan install .. --build=missing -s build_type=Release

# 4. Build project
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G "Ninja"
cmake --build . --parallel

# 5. Run
.\Release\filevault.exe --help
```

---

## 📦 Conan Commands

```powershell
# Install dependencies
conan install .. --build=missing -s build_type=Release

# List installed packages
conan list "*"

# Show package details
conan list botan/3.2.0:*

# Clean cache
conan cache clean "*" --source --build --temp

# Update dependencies
conan install .. --build=missing --update

# Create profile
conan profile detect --force
conan profile show default
```

---

## 🔨 CMake Commands

```powershell
# Configure
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G "Ninja"

# Build
cmake --build . --config Release --parallel

# Clean build
cmake --build . --target clean

# Install
cmake --install .

# Run tests
ctest -C Release --output-on-failure

# Verbose output
cmake --build . --verbose
```

---

## 💻 Development Workflow

### Daily Workflow
```powershell
cd filevault\build

# Make code changes
# ...

# Rebuild (incremental, fast)
cmake --build . --parallel

# Run tests
ctest --output-on-failure

# Run program
.\Release\filevault.exe encrypt test.txt
```

### Clean Rebuild
```powershell
cd filevault
rmdir /s /q build
mkdir build && cd build
conan install .. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G "Ninja"
cmake --build . --parallel
```

### Add New Algorithm
```powershell
# 1. Create files
New-Item include/filevault/algorithms/symmetric/myalgo.hpp
New-Item src/algorithms/symmetric/myalgo.cpp
New-Item tests/algorithms/test_myalgo.cpp

# 2. Implement ICryptoAlgorithm interface
# 3. Register in CryptoEngine
# 4. Add tests with NIST vectors
# 5. Rebuild
cmake --build . --parallel
```

---

## 🧪 Testing Commands

```powershell
# Run all tests
ctest --output-on-failure

# Run specific test
.\tests\Release\test_aes.exe

# Run with filter
.\tests\Release\filevault_tests.exe "[aes]"

# Verbose
ctest -V

# Parallel tests
ctest -j8

# Memory check (if Valgrind available)
ctest -T memcheck
```

---

## 🔐 Critical Security Reminders

### ❌ NEVER DO THIS

```cpp
// ❌ Reusing nonce
static std::vector<uint8_t> nonce(12);
cipher->start(nonce);

// ❌ Hardcoded salt
const std::vector<uint8_t> SALT = {0x01, 0x02, ...};

// ❌ Not clearing password
std::string password = get_password();
// ... use password ...
// password still in memory!

// ❌ Variable-time comparison
if (computed_tag == provided_tag) { }
```

### ✅ ALWAYS DO THIS

```cpp
// ✅ Generate NEW nonce each time
Botan::AutoSeeded_RNG rng;
std::vector<uint8_t> nonce(12);
rng.randomize(nonce.data(), nonce.size());

// ✅ Generate NEW salt each time
std::vector<uint8_t> salt(32);
rng.randomize(salt.data(), salt.size());

// ✅ Use secure types
Botan::secure_vector<uint8_t> key(32);
// Auto-clears on destruction

// ✅ Constant-time comparison
bool valid = Botan::constant_time_compare(a, b, len);
```

---

## 📚 Botan 3.x Quick Reference

### Random Generation
```cpp
Botan::AutoSeeded_RNG rng;
std::vector<uint8_t> data(32);
rng.randomize(data.data(), data.size());
```

### Hashing
```cpp
auto hash = Botan::HashFunction::create("SHA-256");
hash->update(data);
auto result = hash->final();
```

### AES-GCM Encryption
```cpp
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Encryption);
cipher->set_key(key);
Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
cipher->start(nonce);
cipher->finish(buffer); // buffer now has ciphertext + tag
```

### Argon2 KDF
```cpp
auto key = Botan::argon2_generate(
    32,                    // output length
    password.data(),       // password
    password.size(),
    salt.data(),          // salt
    salt.size(),
    65536,                // memory KB
    4,                    // parallelism
    100000                // iterations
);
```

---

## 🐛 Common Issues

| Issue | Solution |
|-------|----------|
| "Botan not found" | `conan install .. --build=missing` |
| "CMake Error" | Check `conan_toolchain.cmake` exists |
| Link errors | Verify `target_link_libraries(... botan::botan)` |
| Slow build | Use Ninja: `-G "Ninja"` |
| Out of memory | Use streaming for large files |
| Tests fail | Check NIST vectors, verify nonce uniqueness |

---

## 📁 Important Files

| File | Purpose |
|------|---------|
| `conanfile.txt` | Dependencies list |
| `CMakeLists.txt` | Build configuration |
| `.github/copilot-instructions.md` | Coding standards |
| `.github/SECURITY_GUIDELINES.md` | Security rules |
| `.github/BOTAN3_REFERENCE.md` | Botan API |
| `.github/TROUBLESHOOTING.md` | Problem solving |

---

## 🎯 Code Templates

### New Algorithm
```cpp
// include/filevault/algorithms/symmetric/myalgo.hpp
class MyAlgo : public core::ICryptoAlgorithm {
public:
    std::string name() const override { return "MyAlgo"; }
    core::AlgorithmType type() const override { return core::AlgorithmType::MY_ALGO; }
    size_t key_size() const override { return 32; }
    
    core::CryptoResult encrypt(std::span<const uint8_t> plaintext,
                              std::span<const uint8_t> key,
                              const core::EncryptionConfig& config) override;
    // ...
};
```

### Test Template
```cpp
TEST_CASE("MyAlgo basic test", "[myalgo]") {
    MyAlgo algo;
    std::vector<uint8_t> plaintext = {'T', 'e', 's', 't'};
    std::vector<uint8_t> key(32, 0xAB);
    
    auto encrypted = algo.encrypt(plaintext, key, config);
    REQUIRE(encrypted.success);
    
    auto decrypted = algo.decrypt(encrypted.data, key, config);
    REQUIRE(decrypted.success);
    REQUIRE(decrypted.data == plaintext);
}
```

---

## 🔍 Debug Commands

```powershell
# Check dependencies
conan list "*"

# Check binary info
dumpbin /dependents .\Release\filevault.exe

# Check symbols
dumpbin /exports .\Release\filevault_lib.lib

# Memory leaks (Windows)
# Use Visual Studio Diagnostic Tools

# Performance profiling
# Use Visual Studio Profiler or Intel VTune
```

---

## 📊 Performance Targets

| Operation | Target | Actual |
|-----------|--------|--------|
| AES-256-GCM | >500 MB/s | TBD |
| Argon2id | <500ms | TBD |
| File I/O | >1 GB/s | TBD |
| Startup | <1 second | TBD |

---

## ✅ Pre-commit Checklist

```powershell
# Before committing:
cmake --build . --parallel           # ✓ Compiles
ctest --output-on-failure           # ✓ Tests pass
clang-format -i src/**/*.cpp        # ✓ Formatted
git add .                            # ✓ Stage changes
git commit -m "feat: add feature"   # ✓ Commit
```

---

## 🎓 Learning Resources

- **Botan Docs**: https://botan.randombit.net/handbook/
- **Conan Docs**: https://docs.conan.io/2/
- **CMake Docs**: https://cmake.org/documentation/
- **C++20**: https://en.cppreference.com/w/cpp/20

---

## 💡 Pro Tips

1. **Use Ninja** for faster builds: `-G "Ninja"`
2. **Parallel builds**: `cmake --build . --parallel`
3. **Cache Conan packages**: Only download once!
4. **Incremental builds**: Very fast after first build
5. **Multiple configs**: Create `build-debug` and `build-release`
6. **VS Code**: Use CMake Tools extension
7. **Copilot**: Let it generate boilerplate
8. **Tests first**: Write tests before implementation

---

**More details in `.github/` documentation!**