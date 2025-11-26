# FileVault Build Guide

Hướng dẫn build FileVault trên các nền tảng khác nhau (Windows, Linux, macOS) với nhiều compiler (MSVC, GCC, Clang).

## 📋 Yêu cầu chung

- **CMake** >= 3.20
- **Conan** 2.x (package manager)
- **Ninja** (build system - khuyến nghị)
- **C++20** compatible compiler

## 🪟 Windows

### Option 1: MSVC (Visual Studio)

**Yêu cầu:**
- Visual Studio 2022/2026 với C++ workload
- Ninja (cài qua `choco install ninja`)

**Bước 1: Tạo Conan Profile**

```powershell
# Tạo thư mục profile
New-Item -ItemType Directory -Force -Path "$env:USERPROFILE\.conan2\profiles"

# Tạo profile MSVC
@"
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=20
compiler.runtime=dynamic
compiler.version=194
os=Windows

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
"@ | Out-File -FilePath "$env:USERPROFILE\.conan2\profiles\msvc" -Encoding utf8NoBOM
```

> **Lưu ý:** Thay `compiler.version=194` theo phiên bản VS của bạn:
> - VS 2022: `193` hoặc `194`
> - VS 2026: `195`

**Bước 2: Build**

```powershell
# Mở Developer Command Prompt hoặc chạy vcvarsall.bat
# Ví dụ với VS 2022:
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 && powershell'

# Cài đặt dependencies
cd D:\code\filevault
mkdir build
cd build
conan install .. --output-folder=. --build=missing -pr msvc

# Configure và Build
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build build/Release --parallel

# Chạy tests
ctest --test-dir build/Release --output-on-failure -j16
```

### Option 2: MinGW GCC (MSYS2 UCRT64)

**Bước 1: Cài đặt MSYS2**

Tải từ https://www.msys2.org/ và cài đặt.

**Bước 2: Cài đặt toolchain (trong MSYS2 UCRT64 Terminal)**

```bash
# Cập nhật hệ thống
pacman -Syu

# Cài đặt toolchain
pacman -S mingw-w64-ucrt-x86_64-toolchain
pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
pacman -S mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-python-pip

# Cài đặt Conan
pip install conan

# Tạo profile
conan profile detect --force
```

**Bước 3: Build**

```bash
cd /d/code/filevault
mkdir -p build && cd build

# Cài đặt dependencies
conan install .. --output-folder=. --build=missing -s compiler=gcc -s compiler.cppstd=20

# Configure
cmake .. -G "Ninja" \
  -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON

# Build
cmake --build . --parallel

# Test
ctest --output-on-failure
```

### Option 3: Clang (MSYS2 UCRT64)

**Cài đặt Clang:**

```bash
pacman -S mingw-w64-ucrt-x86_64-clang
```

**Tạo Conan profile:**

```bash
mkdir -p ~/.conan2/profiles
cat > ~/.conan2/profiles/clang << 'EOF'
[settings]
arch=x86_64
build_type=Release
os=Windows
compiler=clang
compiler.cppstd=20
compiler.version=18
compiler.libcxx=libc++

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
EOF
```

**Build:**

```bash
cd /d/code/filevault
mkdir -p build && cd build
conan install .. --output-folder=. --build=missing -pr clang
cmake .. -G "Ninja" \
  -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DBUILD_TESTS=ON
cmake --build . --parallel
```

---

## 🐧 Linux

### GCC

```bash
# Cài đặt dependencies (Ubuntu/Debian)
sudo apt update
sudo apt install -y build-essential cmake ninja-build python3-pip
pip3 install conan

# Tạo profile
conan profile detect --force

# Hoặc tạo thủ công
mkdir -p ~/.conan2/profiles
cat > ~/.conan2/profiles/default << 'EOF'
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=20
compiler.libcxx=libstdc++11
compiler.version=13
os=Linux
EOF

# Build
cd ~/code/filevault
mkdir -p build && cd build
conan install .. --output-folder=. --build=missing
cmake .. -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON
cmake --build . --parallel $(nproc)

# Test
ctest --output-on-failure -j $(nproc)
```

### Clang

```bash
# Cài đặt Clang
sudo apt install -y clang-17 libc++-17-dev libc++abi-17-dev

# Tạo profile
cat > ~/.conan2/profiles/clang << 'EOF'
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=20
compiler.libcxx=libc++
compiler.version=17
os=Linux
EOF

# Build
export CC=clang-17
export CXX=clang++-17
cd ~/code/filevault
mkdir -p build && cd build
conan install .. --output-folder=. --build=missing -pr clang
cmake .. -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang-17 \
  -DCMAKE_CXX_COMPILER=clang++-17 \
  -DBUILD_TESTS=ON
cmake --build . --parallel $(nproc)
```

---

## 🍎 macOS

```bash
# Cài đặt Xcode Command Line Tools
xcode-select --install

# Cài đặt dependencies
brew install cmake ninja python
pip3 install conan

# Tạo profile
mkdir -p ~/.conan2/profiles
cat > ~/.conan2/profiles/default << 'EOF'
[settings]
arch=armv8
build_type=Release
compiler=apple-clang
compiler.cppstd=20
compiler.libcxx=libc++
compiler.version=16
os=Macos
EOF

# Build
cd ~/code/filevault
mkdir -p build && cd build
conan install .. --output-folder=. --build=missing
cmake .. -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DBUILD_TESTS=ON
cmake --build . --parallel $(sysctl -n hw.ncpu)

# Test
ctest --output-on-failure -j $(sysctl -n hw.ncpu)
```

---

## 🔧 Troubleshooting

### Lỗi "conan_toolchain.cmake not found"

```bash
# Tìm file toolchain
find build -name "conan_toolchain.cmake"

# Sử dụng đường dẫn đầy đủ
cmake .. -DCMAKE_TOOLCHAIN_FILE="$(find build -name 'conan_toolchain.cmake' | head -1)"
```

### Lỗi "Ninja version too old" (MinGW)

Đảm bảo không có `tools.gnu:make_program` trong profile khi dùng Ninja generator.

### Lỗi MSVC Compiler Version

Kiểm tra version MSVC:

```powershell
cl 2>&1 | Select-String "Version"
```

Map version:
- `19.30.x` → `193`
- `19.40.x` → `194`
- `19.50.x` → `195`

### Xóa cache và build lại

```bash
# Xóa build directory
rm -rf build

# Xóa Conan cache (nếu cần)
conan remove "*" -c
```

---

## 📊 Build Matrix

| Platform | Compiler | Status |
|----------|----------|--------|
| Linux | GCC 13 | ✅ |
| Linux | Clang 17 | ✅ |
| Windows | MSVC 2022/2026 | ✅ |
| Windows | MinGW GCC 14 | ✅ |
| macOS | Apple Clang 16 | ✅ |

---

## 🧪 Running Tests

```bash
# Chạy tất cả tests
ctest --test-dir build --output-on-failure

# Chạy test cụ thể
ctest --test-dir build -R "AES_GCM"

# Chạy với verbose output
ctest --test-dir build -V
```

---

## 📦 Dependencies (Managed by Conan)

- **botan** 3.10.0 - Cryptography library
- **fmt** 12.0.0 - Formatting library
- **spdlog** 1.16.0 - Logging library
- **CLI11** 2.6.0 - CLI parser
- **nlohmann_json** 3.12.0 - JSON library
- **zlib** 1.3.1 - Compression
- **bzip3** 1.5.1 - Compression
- **xz_utils** 5.8.1 - LZMA compression
- **indicators** 2.3 - Progress bars
- **tabulate** 1.5 - Table formatting
- **stb** - Image processing
- **Catch2** 3.11.0 - Testing framework
