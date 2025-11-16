# FileVault 🔐

Professional cross-platform file encryption CLI tool with modern C++20 and Botan 3.x.

## Quick Start

```powershell
# Build (Windows 11)
mkdir build && cd build
conan install .. --build=missing -s build_type=Release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G Ninja
cmake --build . --parallel

# Run
.\bin\filevault.exe --help
```

## Features

- ✅ AES-256-GCM encryption
- ✅ Argon2id key derivation
- ✅ Multiple security levels (weak/medium/strong/paranoid)
- ✅ Compression support (zlib, bzip2, lzma)
- ✅ Hash algorithms (SHA-256, SHA-512, BLAKE2b)
- ✅ NIST test vectors
- ✅ Performance benchmarks

## Documentation

See `.github/copilot/` for complete documentation:
- `copilot-instructions.md` - Coding standards
- `architecture.md` - System design
- `botan3.md` - Botan API reference
- `troubleshooting.md` - Common issues

## Usage

```bash
# Encrypt file
filevault encrypt input.txt --password test123 -v

# Decrypt file
filevault decrypt input.txt.fv --password test123

# Hash file
filevault hash document.pdf --algorithm sha256

# List algorithms
filevault list algorithms

# Benchmark
filevault benchmark --algorithm aes-256-gcm
```

## License

MIT
