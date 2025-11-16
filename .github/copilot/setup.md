# FileVault - Windows 11 Setup Guide

## 🎯 Mục tiêu

Setup môi trường development trên Windows 11 với:
- ✅ Visual Studio 2022 hoặc MinGW-w64
- ✅ Conan 2.x để tải packages compiled sẵn
- ✅ CMake để build
- ✅ Không cần build Botan từ source!

---

## 📋 Prerequisites

### 1. Visual Studio 2022 (Recommended) ⭐

**Download:** https://visualstudio.microsoft.com/downloads/

**Chọn workload:**
- ✅ Desktop development with C++
- ✅ C++ CMake tools for Windows
- ✅ C++ Clang tools for Windows (optional)

**Hoặc cài minimal:**
```powershell
# Visual Studio Build Tools 2022 (nhẹ hơn, không có IDE)
winget install Microsoft.VisualStudio.2022.BuildTools
```

### 2. Python 3.8+ (cho Conan)

```powershell
# Cài Python
winget install Python.Python.3.12

# Verify
python --version  # Should be 3.12.x
```

### 3. CMake

```powershell
# Cài CMake
winget install Kitware.CMake

# Verify
cmake --version  # Should be 3.27+
```

### 4. Git

```powershell
# Cài Git
winget install Git.Git

# Verify
git --version
```

---

## 🚀 Quick Setup (Recommended)

### Step 1: Install Conan 2.x

```powershell
# Cài Conan qua pip
pip install conan

# Verify
conan --version  # Should be 2.x

# Detect compiler profile
conan profile detect --force

# Check profile
conan profile show default
```

**Profile sẽ như này:**
```ini
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=20
compiler.runtime=dynamic
compiler.version=193
os=Windows
```

### Step 2: Clone Project

```powershell
# Clone repo
git clone https://github.com/yourusername/filevault.git
cd filevault
```

### Step 3: Install Dependencies với Conan

```powershell
# Tạo build directory
mkdir build
cd build

# Install dependencies (Conan sẽ TẢI packages compiled sẵn!)
conan install .. --build=missing -s build_type=Release

# Nếu muốn Debug build
conan install .. --build=missing -s build_type=Debug
```

**⏰ Lần đầu sẽ lâu (~5-10 phút)** - Conan download:
- Botan 3.2.0 compiled
- fmt, spdlog, CLI11 compiled
- zlib, bzip2, lzma compiled
- Catch2 compiled

**Lần sau nhanh hơn** - Conan cache ở: `~/.conan2/p/`

### Step 4: Configure CMake

```powershell
# Configure với Conan toolchain
cmake .. -DCMAKE_BUILD_TYPE=Release `
         -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake `
         -G "Visual Studio 17 2022"

# Hoặc với Ninja (nhanh hơn)
cmake .. -DCMAKE_BUILD_TYPE=Release `
         -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake `
         -G "Ninja"
```

### Step 5: Build

```powershell
# Build với CMake
cmake --build . --config Release

# Hoặc với MSBuild trực tiếp
msbuild FileVault.sln /p:Configuration=Release

# Hoặc với Ninja
ninja
```

### Step 6: Run

```powershell
# Run executable
.\Release\filevault.exe --help

# Run tests
.\Release\filevault_tests.exe

# Hoặc với CTest
ctest -C Release --output-on-failure
```

---

## 🔧 Alternative: MinGW-w64 (nếu không muốn VS)

### Install MinGW-w64

```powershell
# Cài MSYS2
winget install MSYS2.MSYS2

# Mở MSYS2 MinGW 64-bit terminal
# Cài compiler
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja

# Add to PATH
# System Properties → Environment Variables
# Add: C:\msys64\mingw64\bin
```

### Create MinGW Profile cho Conan

```powershell
# Tạo profile mới
conan profile detect --name mingw

# Edit profile
notepad ~/.conan2/profiles/mingw
```

**mingw profile:**
```ini
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=20
compiler.libcxx=libstdc++11
compiler.version=13
os=Windows

[buildenv]
CC=gcc
CXX=g++
```

### Build với MinGW

```powershell
cd build

# Install với MinGW profile
conan install .. --build=missing -pr:h=mingw -pr:b=mingw

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release `
         -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake `
         -G "MinGW Makefiles"

# Build
cmake --build . -- -j8
```

---

## 🎨 Visual Studio Code Setup

### Extensions cần thiết

```json
{
  "recommendations": [
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "twxs.cmake",
    "GitHub.copilot",
    "ms-vscode.hexeditor"
  ]
}
```

### settings.json

```json
{
  "cmake.configureOnOpen": true,
  "cmake.generator": "Ninja",
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "cmake.configureArgs": [
    "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/build/conan_toolchain.cmake"
  ]
}
```

### tasks.json (Build tasks)

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Conan Install",
      "type": "shell",
      "command": "conan install . --build=missing -s build_type=Release",
      "options": {
        "cwd": "${workspaceFolder}/build"
      },
      "problemMatcher": []
    },
    {
      "label": "CMake Configure",
      "type": "shell",
      "command": "cmake",
      "args": [
        "..",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake",
        "-G", "Ninja"
      ],
      "options": {
        "cwd": "${workspaceFolder}/build"
      },
      "dependsOn": ["Conan Install"]
    },
    {
      "label": "Build",
      "type": "shell",
      "command": "cmake",
      "args": ["--build", ".", "--config", "Release"],
      "options": {
        "cwd": "${workspaceFolder}/build"
      },
      "group": {
        "kind": "build",
        "isDefault": true
      }
    }
  ]
}
```

---

## 🐛 Troubleshooting

### 1. "Conan not found"

```powershell
# Check Python Scripts in PATH
$env:PATH += ";$env:USERPROFILE\AppData\Local\Programs\Python\Python312\Scripts"

# Hoặc reinstall
pip install --upgrade conan
```

### 2. "MSVC not found"

```powershell
# Option 1: Cài Visual Studio 2022
# Option 2: Cài Build Tools
# Option 3: Dùng MinGW-w64

# Kiểm tra
where cl.exe  # Should show MSVC compiler path
```

### 3. "Botan 3.2.0 not found in Conan Center"

```powershell
# Kiểm tra available versions
conan search botan --remote=conancenter

# Nếu không có 3.2.0, dùng version gần nhất
# Edit conanfile.txt:
botan/3.1.1  # hoặc version có sẵn
```

### 4. "CMake can't find packages"

```powershell
# Đảm bảo dùng Conan toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

# Check file tồn tại
dir conan_toolchain.cmake
```

### 5. "Link errors với Botan"

```powershell
# Check trong CMakeLists.txt:
find_package(botan REQUIRED)
target_link_libraries(your_target PRIVATE botan::botan)

# NOT: botan-3 or botan::botan-3
# Conan auto-handles versioning
```

### 6. Build quá lâu

```powershell
# Dùng Ninja (nhanh hơn MSBuild)
cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

# Build parallel
cmake --build . -- -j8  # 8 cores
```

---

## 🔍 Verify Installation

### Test Botan

```cpp
// test_botan.cpp
#include <botan/version.h>
#include <iostream>

int main() {
    std::cout << "Botan version: " << Botan::version_string() << "\n";
    std::cout << "Major: " << BOTAN_VERSION_MAJOR << "\n";
    
    if (BOTAN_VERSION_MAJOR >= 3) {
        std::cout << "✓ Botan 3.x OK\n";
    }
    return 0;
}
```

```powershell
# Compile test
cd build
cmake --build . --target test_botan
.\Release\test_botan.exe
```

### Check Dependencies

```powershell
# List installed packages
conan list "*"

# Show package info
conan list botan/3.2.0:*

# Check binary
conan list botan/3.2.0:* --format=compact
```

---

## 📊 Performance Tips

### 1. Use Ninja Generator

```powershell
# Ninja nhanh hơn MSBuild ~30-50%
cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
```

### 2. Parallel Build

```powershell
# MSBuild
msbuild FileVault.sln /m:8 /p:Configuration=Release

# Ninja
ninja -j8

# CMake (auto-detect cores)
cmake --build . --parallel
```

### 3. Precompiled Headers

```cmake
# In CMakeLists.txt
target_precompile_headers(filevault_lib PRIVATE
    <botan/version.h>
    <botan/auto_rng.h>
    <botan/cipher_mode.h>
)
```

### 4. Cache Conan Packages

```powershell
# Packages cached at:
echo $env:USERPROFILE\.conan2\p\

# Nếu build nhiều projects, chỉ download 1 lần!
```

---

## 🏗️ Project Structure trên Windows

```
C:\Projects\filevault\
├── .git\
├── .github\                   # Instructions cho Copilot
├── .vscode\                   # VS Code config
├── build\                     # Build directory
│   ├── conan_toolchain.cmake  # Generated by Conan
│   ├── CMakeCache.txt
│   └── Release\
│       └── filevault.exe      # Your executable
├── include\
├── src\
├── tests\
├── CMakeLists.txt
├── conanfile.txt              # Dependencies
└── README.md
```

---

## 🎯 Recommended Workflow

### First Time Setup (15-20 minutes)

```powershell
# 1. Install prerequisites (5 min)
winget install Microsoft.VisualStudio.2022.BuildTools
winget install Python.Python.3.12
winget install Kitware.CMake
pip install conan

# 2. Clone and setup (10 min)
git clone https://github.com/yourusername/filevault.git
cd filevault
mkdir build
cd build

# 3. Install dependencies - CONAN TẢI COMPILED PACKAGES! (5-10 min first time)
conan install .. --build=missing -s build_type=Release

# 4. Configure and build (2-3 min)
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G "Ninja"
cmake --build . --parallel
```

### Daily Development (< 1 minute)

```powershell
cd filevault\build

# Code changes
# ...

# Rebuild (incremental, very fast)
cmake --build . --parallel

# Run
.\Release\filevault.exe
```

### Clean Rebuild

```powershell
# Remove build directory
cd filevault
rmdir /s /q build
mkdir build
cd build

# Reinstall dependencies (fast, from cache)
conan install .. --build=missing -s build_type=Release

# Rebuild
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G "Ninja"
cmake --build . --parallel
```

---

## 💡 Pro Tips

### 1. Multiple Build Configs

```powershell
# Debug build
mkdir build-debug
cd build-debug
conan install .. --build=missing -s build_type=Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .

# Release build
mkdir build-release
cd build-release
conan install .. --build=missing -s build_type=Release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .
```

### 2. Conan Cache Management

```powershell
# Check cache size
conan cache path

# Clean old packages
conan cache clean "*" --source --build --temp

# Remove specific package
conan remove "botan/3.1.0"
```

### 3. Update Dependencies

```powershell
# Update conanfile.txt với versions mới
# Then:
cd build
conan install .. --build=missing --update
cmake ..
cmake --build .
```

---

## 🖥️ Máy thật vs Máy ảo?

### Máy thật (Recommended) ⭐⭐⭐⭐⭐

**Pros:**
- ✅ Native performance
- ✅ Full hardware support (AES-NI)
- ✅ Dễ debug
- ✅ Dễ cài đặt
- ✅ No virtualization overhead

**Cons:**
- ❌ Có thể "làm bẩn" system
- ❌ Khó rollback nếu có vấn đề

**→ Dùng máy thật!** Vì:
1. Crypto cần performance (AES-NI)
2. Build nhanh hơn nhiều
3. Development trải nghiệm tốt hơn

### Máy ảo (Optional)

**Khi nào dùng:**
- ✅ Test trên Linux/other OS
- ✅ Isolated environment
- ✅ CI/CD testing

**Không nên:**
- ❌ Development chính trên VM (chậm)
- ❌ Performance testing trên VM (không chính xác)

### WSL2 (Middle Ground)

```powershell
# Enable WSL2
wsl --install

# Install Ubuntu
wsl --install -d Ubuntu-22.04

# Use WSL for Linux builds
cd /mnt/c/Projects/filevault
# ... build on Linux
```

**Pros:**
- ✅ Linux environment
- ✅ Native Windows file access
- ✅ Better performance than full VM

---

## ✅ Final Checklist

Sau khi setup xong:

- [ ] `conan --version` shows 2.x
- [ ] `cmake --version` shows 3.27+
- [ ] `python --version` shows 3.8+
- [ ] Can compile test program
- [ ] All tests pass
- [ ] No linker errors
- [ ] Botan 3.x detected

**Ready to code! 🚀**

---

## 📚 References

- Conan Docs: https://docs.conan.io/2/
- CMake Docs: https://cmake.org/documentation/
- Botan Windows: https://botan.randombit.net/handbook/building.html#windows
- Visual Studio: https://visualstudio.microsoft.com/

---

**Questions?** Check TROUBLESHOOTING.md or open an issue!


[requires]
# Cryptography (C++20 compatible)
botan/3.2.0

# Compression
zlib/1.3.1
bzip2/1.0.8
xz_utils/5.4.5

# Testing
catch2/3.5.2

# Formatting and I/O
fmt/10.2.1              # Modern formatting
spdlog/1.13.0           # Fast logging
cli11/2.4.1             # Modern CLI parsing

# JSON support
nlohmann_json/3.11.3

# UI/UX utilities
indicators/2.3          # Progress bars
tabulate/1.5            # Pretty tables

[tool_requires]
cmake/3.27.7

[generators]
CMakeDeps
CMakeToolchain

[options]
# Botan options
botan/*:shared=False
botan/*:with_bzip2=True
botan/*:with_zlib=True
botan/*:with_lzma=True
botan/*:enable_modules=aes,gcm,chacha20poly1305,argon2,pbkdf2,sha2,sha3,blake2

# Compression options
zlib/*:shared=False
bzip2/*:shared=False
xz_utils/*:shared=False

# Other options
fmt/*:shared=False
spdlog/*:shared=False

[layout]
cmake_layout

[options]
botan:shared=False
botan:enable_modules=auto
botan:amalgamation=True
zlib:shared=False
bzip2:shared=False
xz_utils:shared=False

[layout]
cmake_layout

