# FileVault 🔐

**Professional cross-platform file encryption CLI tool** built with modern C++20 and Botan 3.x cryptographic library.

[![CI](https://github.com/vuongdat67/NT140.Q11.ANTT-Group15/actions/workflows/ci.yml/badge.svg)](https://github.com/vuongdat67/NT140.Q11.ANTT-Group15/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## ✨ Features

### 🔒 Modern Encryption
- **AEAD Ciphers**: AES-GCM (128/192/256), ChaCha20-Poly1305, Serpent-GCM, Twofish-GCM
- **International Standards**: Camellia-GCM, ARIA-GCM (Korea), SM4-GCM (China)
- **Legacy Support**: AES-CBC/CTR/CFB/OFB/XTS, 3DES (for compatibility)
- **Asymmetric**: RSA (2048/3072/4096), ECC (P-256/P-384/P-521)
- **Classical** (educational): Caesar, Vigenère, Playfair, Hill, Substitution

### 🔑 Key Derivation
- **Argon2id** - Memory-hard, recommended
- **Scrypt** - Memory-hard alternative
- **PBKDF2** (SHA-256/SHA-512) - Legacy compatible

### 📦 Compression
- **ZLIB** - Fast, good ratio
- **LZMA** - Best ratio, slower
- **BZIP2** - Balanced (coming soon)

### 🎨 Additional Features
- **Steganography** - Hide data in images (LSB)
- **Archive** - Encrypt multiple files
- **Hashing** - SHA-256, SHA-512, SHA-3, BLAKE2b, BLAKE3
- **Benchmarks** - Performance testing

---

## 🚀 Quick Start

### Windows (MSVC)
```powershell
# Setup
.\scripts\setup-msvc.ps1

# Build
.\scripts\build-msvc.ps1 -Test
```

### Windows (MinGW/MSYS2)
```bash
# In MSYS2 UCRT64 terminal
./scripts/setup-mingw.sh
./scripts/build-mingw.sh -t
```

### Linux
```bash
./scripts/setup-linux.sh    # or --clang for Clang
./scripts/build-linux.sh -t
```

### macOS
```bash
./scripts/setup-macos.sh
./scripts/build-macos.sh -t
```

---

## 📖 Usage

### Basic Encryption
```bash
# Encrypt with AES-256-GCM (default)
filevault encrypt secret.txt

# Decrypt
filevault decrypt secret.txt.fvlt

# With options
filevault encrypt data.zip -a chacha20-poly1305 -s paranoid --compression lzma
```

### Mode Presets
```bash
filevault encrypt file.txt --mode basic      # AES-128-GCM, fast
filevault encrypt file.txt --mode standard   # AES-256-GCM, balanced
filevault encrypt file.txt --mode advanced   # ChaCha20-Poly1305, max security
```

### Asymmetric Encryption
```bash
# Generate key pair
filevault keygen --algorithm rsa-4096 --output mykey

# Encrypt with public key
filevault encrypt secret.txt --pubkey mykey.pub

# Decrypt with private key
filevault decrypt secret.txt.fvlt --privkey mykey.pem
```

### Steganography
```bash
# Hide data in image
filevault stego embed message.txt cover.png -o hidden.png

# Extract hidden data
filevault stego extract hidden.png -o recovered.txt

# Check capacity
filevault stego capacity photo.jpg
```

### Archive
```bash
# Create encrypted archive
filevault archive create documents/ -o backup.fva

# Extract archive
filevault archive extract backup.fva -o restored/

# List contents
filevault archive list backup.fva
```

### Hash
```bash
filevault hash document.pdf                    # SHA-256 (default)
filevault hash file.iso --algorithm blake2b   # BLAKE2b
filevault hash ./folder/ --recursive          # All files
```

### Benchmark
```bash
filevault benchmark                            # All algorithms
filevault benchmark --algorithm aes-256-gcm   # Specific
filevault benchmark --json -o results.json    # Export
```

### List & Info
```bash
filevault list algorithms    # Supported algorithms
filevault list kdfs          # Key derivation functions
filevault info encrypted.fvlt  # File metadata
```

---

## 🏗️ Building

### Requirements
- **C++20** compatible compiler
- **CMake** >= 3.20
- **Conan** 2.x
- **Ninja** (recommended)

### Supported Platforms

| Platform | Compiler | Status |
|----------|----------|--------|
| Linux | GCC 13+ | ✅ |
| Linux | Clang 17+ | ✅ |
| Windows | MSVC 2022/2026 | ✅ |
| Windows | MinGW GCC 14+ | ✅ |
| macOS | Apple Clang 16+ | ✅ |

### Manual Build
```bash
mkdir build && cd build
conan install .. --output-folder=. --build=missing
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build build/Release --parallel
```

See [docs/BUILD.md](docs/BUILD.md) for detailed instructions.

---

## 📊 Algorithm Comparison

### Symmetric (AEAD - Authenticated Encryption)

| Algorithm | Key Size | Speed | Security | Use Case |
|-----------|----------|-------|----------|----------|
| AES-256-GCM | 256-bit | ⚡⚡⚡ | ⭐⭐⭐⭐⭐ | General purpose |
| ChaCha20-Poly1305 | 256-bit | ⚡⚡⚡ | ⭐⭐⭐⭐⭐ | Mobile, no AES-NI |
| Serpent-256-GCM | 256-bit | ⚡⚡ | ⭐⭐⭐⭐⭐ | High security |
| Camellia-256-GCM | 256-bit | ⚡⚡⚡ | ⭐⭐⭐⭐⭐ | Japan standard |
| ARIA-256-GCM | 256-bit | ⚡⚡⚡ | ⭐⭐⭐⭐⭐ | Korea standard |
| SM4-GCM | 128-bit | ⚡⚡⚡ | ⭐⭐⭐⭐ | China standard |

### Key Derivation

| KDF | Memory | Speed | Resistance |
|-----|--------|-------|------------|
| Argon2id | 64MB+ | Slow | GPU, ASIC |
| Scrypt | 32MB+ | Slow | GPU |
| PBKDF2 | Minimal | Fast | Brute force only |

---

## 🧪 Testing

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
ctest --test-dir build -R "AES_GCM"

# With verbose output
ctest --test-dir build -V
```

### Test Categories
- **Unit tests** - Individual components
- **Integration tests** - Full encrypt/decrypt flow
- **Security tests** - Nonce uniqueness, timing attacks
- **NIST vectors** - Standard test vectors

---

## 📁 Project Structure

```
filevault/
├── include/filevault/     # Headers
│   ├── algorithms/        # Crypto algorithms
│   ├── cli/               # CLI commands
│   ├── core/              # Core types & engine
│   └── utils/             # Utilities
├── src/                   # Implementation
├── tests/                 # Test suites
├── scripts/               # Build scripts
└── docs/                  # Documentation
```

---

## 🔧 Configuration

### Security Levels

| Level | KDF Iterations | Memory | Description |
|-------|----------------|--------|-------------|
| weak | 3 | 64MB | Fast, testing |
| medium | 10 | 128MB | Balanced |
| strong | 20 | 256MB | Recommended |
| paranoid | 50 | 512MB | Maximum |

### Config File
```bash
filevault config set default-algorithm aes-256-gcm
filevault config set default-kdf argon2id
filevault config show
```

---

## 📚 Documentation

- [BUILD.md](docs/BUILD.md) - Build instructions
- [.github/copilot/](/.github/copilot/) - Architecture & coding standards

---

## 🤝 Contributing

1. Fork the repository
2. Create feature branch: `git checkout -b feature/amazing`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push: `git push origin feature/amazing`
5. Open Pull Request

---

## 📜 License

MIT License - see [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

- [Botan](https://botan.randombit.net/) - Crypto library
- [CLI11](https://github.com/CLIUtils/CLI11) - CLI parser
- [spdlog](https://github.com/gabime/spdlog) - Logging
- [indicators](https://github.com/p-ranav/indicators) - Progress bars
