# FileVault - Quick Command Reference

## 🔧 Setup Commands

### Python & Conan Setup
```powershell
python3 --version
python -m venv .venv
pip install conan

# Add to PATH if needed
$env:Path += ";C:\Users\vuong\AppData\Local\Programs\Python\Python310\Scripts"
[Environment]::SetEnvironmentVariable("Path", $env:Path, "User")

conan --version
conan profile detect --force
```

### Compiler Setup
```powershell
# Visual Studio
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# Or MSYS2/MinGW
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-toolchain
pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

### Build Commands
```powershell
# Conan dependencies
conan install . --output-folder=build --build=missing -s compiler=gcc -s compiler.cppstd=20

# CMake configure
cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="build\build\Release\generators\conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

---

## 🚀 FileVault Commands

### Encryption
```bash
filevault encrypt input.txt                                      # Basic
filevault encrypt secret.pdf --security strong                   # Strong security
filevault encrypt data.zip --algorithm aes-256-gcm --compress    # With compression
filevault encrypt *.txt --batch --password "pass123"             # Batch
```

### Decryption
```bash
filevault decrypt encrypted.fv                                   # Basic
filevault decrypt secret.fv -o output.txt                        # With output
filevault decrypt *.fv --batch --password "pass123"              # Batch
```

### Hashing
```bash
filevault hash document.pdf                                      # SHA-256
filevault hash file.zip --algorithm sha512                       # SHA-512
filevault hash input.txt --algorithm sha256,blake2b              # Multiple
filevault hash ./docs/ --recursive --output checksums.txt        # Recursive
```

### Benchmarks
```bash
filevault benchmark                                              # All algorithms
filevault benchmark --algorithm aes-256-gcm                      # Specific
filevault benchmark --compare aes-256-gcm,serpent-256-gcm        # Compare
filevault benchmark --kdf argon2id,scrypt                        # KDF test
```

### Steganography
```bash
filevault stego embed secret.txt cover.png -o stego.png          # Hide
filevault stego embed msg.txt photo.png --encrypt --password pw  # Hide + encrypt
filevault stego extract stego.png -o recovered.txt               # Extract
filevault stego capacity cover.png                               # Check capacity
```

### Info & List
```bash
filevault info encrypted.fv                                      # Show metadata
filevault info secret.fv --format json                           # JSON format
filevault list                                                   # All algorithms
filevault list --category symmetric                              # Symmetric only
```

---

## 🧪 Testing Commands

```powershell
# Run all tests
ctest --output-on-failure

# Specific test
.\tests\Release\test_aes.exe

# With filter
.\tests\Release\filevault_tests.exe "[serpent]"

# Parallel
ctest -j8

# Memory check
valgrind --leak-check=full .\bin\filevault encrypt test.txt
```

---

## 🔍 Development Commands

```powershell
# Encoding check & fix
.\scripts\normalize-encoding.ps1 -DryRun
.\scripts\normalize-encoding.ps1

# Format code
clang-format -i src/**/*.cpp include/**/*.hpp

# Rebuild
rm -r build; mkdir build; cd build
conan install .. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G Ninja
cmake --build . --parallel
```

---

## 📊 Performance Testing

```bash
# Quick test
filevault benchmark --size 10MB

# Full comparison
filevault benchmark --compare aes-128-gcm,aes-256-gcm,serpent-256-gcm --size 100MB --iterations 10

# Measure time
Measure-Command { filevault encrypt largefile.bin --password test }
```

---

## ✅ Pre-commit Checklist

```powershell
# Build, test, format check
cmake --build build --parallel
ctest --test-dir build --output-on-failure
clang-format --dry-run --Werror src/**/*.cpp

# Or one command
cmake --build build --parallel && ctest --test-dir build --output-on-failure
```

---

## 📚 Documentation Links

- Full command reference: `docs/commands.md`
- Recent updates: `docs/RECENT_UPDATES.md`
- Architecture: `.github/copilot/architecture.md`
- Troubleshooting: `.github/copilot/troubleshooting.md`
- Botan API: `.github/copilot/botan3.md`

---

**Last Updated:** 2024-11-20