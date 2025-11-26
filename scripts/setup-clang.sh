#!/bin/bash
# FileVault - Clang/MSYS2 Setup Script
# Usage: ./scripts/setup-clang.sh

set -e

echo "========================================"
echo " FileVault - Clang/MSYS2 Setup"
echo "========================================"

# Check if running in MSYS2
if [[ -z "$MSYSTEM" ]]; then
    echo "❌ This script must be run in MSYS2 terminal (UCRT64 recommended)"
    exit 1
fi

echo "Running in: $MSYSTEM"

# Update and install Clang
echo "Installing Clang toolchain..."
pacman -S --needed --noconfirm \
    mingw-w64-ucrt-x86_64-clang \
    mingw-w64-ucrt-x86_64-lld \
    mingw-w64-ucrt-x86_64-libc++ \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-python \
    mingw-w64-ucrt-x86_64-python-pip

echo "✓ Clang toolchain installed"

# Install Conan
pip install --break-system-packages conan
echo "✓ Conan installed"

# Get Clang version
CLANG_VERSION=$(clang --version | grep -oP '\d+' | head -1)

# Create Conan profile
mkdir -p ~/.conan2/profiles
cat > ~/.conan2/profiles/clang << EOF
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=20
compiler.version=${CLANG_VERSION}
compiler.libcxx=libc++
os=Windows

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
EOF

echo "✓ Created Conan profile for Clang ${CLANG_VERSION}"

echo ""
echo "========================================"
echo " Setup Complete!"
echo "========================================"
echo ""
echo "To build with Clang:"
echo "  mkdir -p build && cd build"
echo "  conan install .. --output-folder=. --build=missing -pr clang"
echo "  cmake .. -G Ninja \\"
echo "    -DCMAKE_C_COMPILER=clang \\"
echo "    -DCMAKE_CXX_COMPILER=clang++ \\"
echo "    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \\"
echo "    -DBUILD_TESTS=ON"
echo "  ninja"
