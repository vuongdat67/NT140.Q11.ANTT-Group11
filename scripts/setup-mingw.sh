#!/bin/bash
# FileVault - MinGW/MSYS2 Setup Script
# Usage: ./scripts/setup-mingw.sh

set -e

echo "========================================"
echo " FileVault - MinGW/MSYS2 Setup"
echo "========================================"

# Check if running in MSYS2
if [[ -z "$MSYSTEM" ]]; then
    echo "❌ This script must be run in MSYS2 terminal (UCRT64 recommended)"
    echo "   Open: MSYS2 UCRT64 from Start Menu"
    exit 1
fi

echo "Running in: $MSYSTEM"

# Update system
echo "Updating MSYS2..."
pacman -Syu --noconfirm

# Install toolchain
echo "Installing toolchain..."
pacman -S --needed --noconfirm \
    mingw-w64-ucrt-x86_64-toolchain \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-python \
    mingw-w64-ucrt-x86_64-python-pip

echo "✓ Toolchain installed"

# Install Conan
echo "Installing Conan..."
pip install --break-system-packages conan
echo "✓ Conan installed"

# Create/Update Conan profile
echo "Creating Conan profile..."
GCC_VERSION=$(gcc -dumpversion | cut -d. -f1)

mkdir -p ~/.conan2/profiles
cat > ~/.conan2/profiles/default << EOF
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=20
compiler.libcxx=libstdc++11
compiler.version=${GCC_VERSION}
os=Windows

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
EOF

echo "✓ Created Conan profile (GCC ${GCC_VERSION})"

echo ""
echo "========================================"
echo " Setup Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo "  ./scripts/build-mingw.sh"
echo ""
echo "Or manually:"
echo "  mkdir -p build && cd build"
echo "  conan install .. --output-folder=. --build=missing"
echo "  cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DBUILD_TESTS=ON"
echo "  ninja"
