# FileVault GUI

Modern desktop application for FileVault encryption tool built with Tauri, React, and TypeScript.

## Features

- ✅ **Encryption/Decryption** - AES-256-GCM, ChaCha20-Poly1305, Camellia
- ✅ **Hash** - SHA256, SHA512, SHA3, BLAKE2b, BLAKE3
- ✅ **Key Generation** - RSA, ECC keypairs
- ✅ **Digital Signatures** - Sign and verify files
- ✅ **Steganography** - Hide/extract data in images
- ✅ **Archive** - Create/extract compressed archives
- ✅ **Compression** - LZMA, ZLIB, BZIP2, BZIP3
- 🎨 **Modern UI** - Dark mode, responsive design
- 🚀 **Fast** - Native performance with Rust backend
- 🔒 **Secure** - No data sent online, everything local

## Prerequisites

- Node.js 20+ 
- Rust (latest stable)
- Windows (primary target)

## Installation & Development

### 1. Install dependencies

```bash
npm install
```

### 2. Copy FileVault CLI executable

```powershell
Copy-Item "..\build\build\Release\bin\release\filevault.exe" "src-tauri\bin\" -Force
```

### 3. Run development mode

```bash
npm run tauri dev
```

### 4. Build for production

```bash
npm run tauri build
```

## Tech Stack

- **Frontend**: React 18, TypeScript, TailwindCSS
- **Backend**: Rust (Tauri 2)
- **UI Components**: Custom components with shadcn/ui styling
- **Routing**: React Router
- **Build**: Vite

## Recommended IDE Setup

- [VS Code](https://code.visualstudio.com/) + [Tauri](https://marketplace.visualstudio.com/items?itemName=tauri-apps.tauri-vscode) + [rust-analyzer](https://marketplace.visualstudio.com/items?itemName=rust-lang.rust-analyzer)
