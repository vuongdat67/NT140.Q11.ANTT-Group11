# FileVault Extension for Visual Studio Code

> Secure file encryption directly in VS Code using the FileVault cryptographic toolkit with **Post-Quantum Cryptography** support.

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🔐 **Encrypt Files** | Right-click any file to encrypt with 47+ cryptographic algorithms |
| 🔓 **Decrypt Files** | Right-click `.fvlt` files to decrypt with password |
| ℹ️ **File Info** | View encryption metadata (algorithm, timestamps, security level) |
| 🔑 **Key Generation** | Generate RSA, ECC, Kyber, and Dilithium key pairs |
| 📊 **Benchmark** | Run performance benchmarks on cryptographic algorithms |
| #️⃣ **Hash Files** | Calculate file hashes (SHA-256, SHA-512, BLAKE2, SHA3) |
| 📋 **List Algorithms** | View all 47+ supported encryption algorithms |

## 🔒 Supported Algorithms

### 🛡️ AEAD (Authenticated Encryption) - Recommended

| Algorithm | Key Size | Security | Notes |
|-----------|----------|----------|-------|
| **AES-256-GCM** | 256-bit | Maximum | ⭐ Recommended - NIST Standard |
| AES-128-GCM | 128-bit | Good | Fast, NIST Standard |
| ChaCha20-Poly1305 | 256-bit | Maximum | Software-optimized |
| Serpent-256-GCM | 256-bit | Maximum | AES Finalist |
| Twofish-256-GCM | 256-bit | Maximum | AES Finalist |
| Camellia-256-GCM | 256-bit | Maximum | Japan (CRYPTREC) |
| ARIA-256-GCM | 256-bit | Maximum | Korea (KS X 1213) |
| SM4-GCM | 128-bit | Strong | China (GB/T 32907) |

### 🚀 Post-Quantum Cryptography (NIST Selected)

| Algorithm | Type | NIST Level | Security |
|-----------|------|------------|----------|
| **Kyber-1024-Hybrid** | KEM + AES-GCM | Level 5 | 256-bit quantum-safe |
| Kyber-768-Hybrid | KEM + AES-GCM | Level 3 | 192-bit quantum-safe |
| Kyber-512-Hybrid | KEM + AES-GCM | Level 1 | 128-bit quantum-safe |
| Dilithium-5 | Signature | Level 5 | Maximum security |
| Dilithium-3 | Signature | Level 3 | Balanced |
| Dilithium-2 | Signature | Level 2 | Fast signing |

### 🔐 Block Cipher Modes

| Algorithm | Key Size | Mode | Notes |
|-----------|----------|------|-------|
| AES-256-CBC | 256-bit | Block | With HMAC authentication |
| AES-256-CTR | 256-bit | Stream | Counter mode |
| AES-256-XTS | 512-bit | Disk | Storage/disk encryption |
| AES-256-CFB | 256-bit | Stream | Self-synchronizing |
| AES-256-OFB | 256-bit | Stream | Pre-computed keystream |

### 🔑 Asymmetric Encryption

| Algorithm | Key Size | Security | Use Case |
|-----------|----------|----------|----------|
| RSA-4096 | 4096-bit | Maximum | High security |
| RSA-3072 | 3072-bit | Strong | Recommended minimum |
| ECC-P521 | 521-bit | Maximum | ECDH + AES-GCM hybrid |
| ECC-P384 | 384-bit | Strong | 192-bit security |

### 📚 Classical Ciphers (Educational)

> ⚠️ **For educational purposes only - NOT secure!**

- Caesar, Vigenère, Playfair, Hill, Substitution

### 🔒 Hash Functions

| Algorithm | Output | Notes |
|-----------|--------|-------|
| SHA-256 | 256-bit | Standard |
| SHA-512 | 512-bit | Stronger |
| SHA3-256 | 256-bit | NIST SHA-3 |
| SHA3-512 | 512-bit | NIST SHA-3 |
| BLAKE2b | 512-bit | Modern, fastest |

### 🛡️ Security Levels (Key Derivation)

| Level | Memory | Time | Use Case |
|-------|--------|------|----------|
| weak | 4 MB | ~2ms | Testing only |
| **medium** | 16 MB | ~10ms | ⭐ Recommended |
| strong | 64 MB | ~30ms | Sensitive data |
| paranoid | 128 MB | ~60ms | Top secret |

## 📦 Installation

### Option 1: From VSIX (Recommended)

1. Download `filevault-1.0.0.vsix` from [Releases](https://github.com/vuongdat67/NT140.Q11.ANTT-Group11/releases)
2. Open VS Code
3. Press `Ctrl+Shift+P` → `Extensions: Install from VSIX...`
4. Select the downloaded `.vsix` file

### Option 2: Build from Source

```bash
cd vscode
npm install
npm run compile
npx @vscode/vsce package
code --install-extension filevault-1.0.0.vsix
```

## ⚙️ Configuration

The extension **auto-detects** FileVault executable on activation! 

Searched locations:
- Workspace build folder (`build/build/Release/bin/release/`)
- Workspace `dist/` folder
- Program Files (`C:\Program Files\FileVault\`)
- LocalAppData (`%LOCALAPPDATA%\FileVault\`)
- Linux/macOS (`/usr/local/bin/`, `~/.local/bin/`)

### Manual Configuration

If auto-detection fails:

1. **Command Palette**: `Ctrl+Shift+P` → `FileVault: Set Executable Path`
2. **Settings**: `Ctrl+,` → Search `filevault.executablePath`

### All Settings

| Setting | Description | Default |
|---------|-------------|---------|
| `filevault.executablePath` | Path to FileVault executable | Auto-detected |
| `filevault.defaultAlgorithm` | Default encryption algorithm | `aes-256-gcm` |
| `filevault.defaultSecurityLevel` | Security level for KDF | `medium` |
| `filevault.showNotifications` | Show success notifications | `true` |
| `filevault.deleteOriginalAfterEncrypt` | Delete original after encrypt | `false` |

## 🚀 Usage

### Encrypt a File

1. Right-click a file in Explorer
2. Select **FileVault: Encrypt File**
3. Choose algorithm (AES-256-GCM recommended)
4. Select security level
5. Enter password (minimum 8 characters)
6. Encrypted file created with `.fvlt` extension

### Decrypt a File

1. Right-click a `.fvlt` file in Explorer
2. Select **FileVault: Decrypt File**
3. Enter the password
4. Original file restored

### Generate Keys (Asymmetric/PQC)

1. `Ctrl+Shift+P` → **FileVault: Generate Key Pair**
2. Select algorithm:
   - RSA-4096 (Traditional)
   - Kyber-1024 (Post-Quantum)
   - Dilithium-5 (PQ Signatures)
   - ECC-P521 (Elliptic Curve)
3. Choose output folder
4. Key files created: `name.pub` and `name.key`

### Run Benchmark

1. `Ctrl+Shift+P` → **FileVault: Run Benchmark**
2. Select benchmark type:
   - All Algorithms
   - Symmetric Only
   - Asymmetric Only
   - Post-Quantum Only
3. View results in Output panel

### List All Algorithms

1. `Ctrl+Shift+P` → **FileVault: List All Algorithms**
2. View complete algorithm list in Output panel

### Calculate Hash

1. Right-click any file
2. Select **FileVault: Calculate Hash**
3. Choose algorithm (SHA-256, SHA-512, BLAKE2b, SHA3)
4. Copy hash to clipboard

## 📋 Commands

| Command | Description |
|---------|-------------|
| `FileVault: Encrypt File` | Encrypt selected file |
| `FileVault: Decrypt File` | Decrypt .fvlt file |
| `FileVault: Show File Info` | View encryption metadata |
| `FileVault: Generate Key Pair` | Create RSA/ECC/Kyber/Dilithium keys |
| `FileVault: Calculate Hash` | Hash file with SHA/BLAKE2 |
| `FileVault: Run Benchmark` | Performance benchmarks |
| `FileVault: List All Algorithms` | Show all 47+ algorithms |
| `FileVault: Set Executable Path` | Configure FileVault path |

## 🔧 Requirements

- Visual Studio Code 1.85.0+
- FileVault CLI installed
  - Download from [Releases](https://github.com/vuongdat67/NT140.Q11.ANTT-Group11/releases)
  - Or build from source

## 📄 License

MIT License - see [LICENSE](LICENSE)

## 🤝 Contributing

Contributions welcome! Please see the main [repository](https://github.com/vuongdat67/NT140.Q11.ANTT-Group11).

---

**FileVault** - Secure your files with modern cryptography including Post-Quantum protection! 🔐
