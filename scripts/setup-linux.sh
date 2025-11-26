#!/bin/bash
# FileVault - Linux Setup Script
# Usage: ./scripts/setup-linux.sh [--clang]

set -e

USE_CLANG=false
if [[ "$1" == "--clang" ]]; then
    USE_CLANG=true
fi

echo "========================================"
echo " FileVault - Linux Setup"
echo "========================================"

# Detect package manager
if command -v apt-get &> /dev/null; then
    PKG_MANAGER="apt"
elif command -v dnf &> /dev/null; then
    PKG_MANAGER="dnf"
elif command -v pacman &> /dev/null; then
    PKG_MANAGER="pacman"
else
    echo "❌ Unsupported package manager"
    exit 1
fi

echo "Package manager: $PKG_MANAGER"

# Install dependencies
case $PKG_MANAGER in
    apt)
        sudo apt-get update
        sudo apt-get install -y build-essential cmake ninja-build python3-pip
        if $USE_CLANG; then
            sudo apt-get install -y clang-17 libc++-17-dev libc++abi-17-dev lld-17
        else
            sudo apt-get install -y gcc-13 g++-13
        fi
        ;;
    dnf)
        sudo dnf install -y gcc gcc-c++ cmake ninja-build python3-pip
        if $USE_CLANG; then
            sudo dnf install -y clang clang-tools-extra libcxx libcxx-devel lld
        fi
        ;;
    pacman)
        sudo pacman -S --needed base-devel cmake ninja python python-pip
        if $USE_CLANG; then
            sudo pacman -S --needed clang lld libc++
        fi
        ;;
esac

echo "✓ Build tools installed"

# Install Conan
pip3 install --user conan
echo "✓ Conan installed"

# Detect compiler version
if $USE_CLANG; then
    COMPILER="clang"
    COMPILER_VERSION=$(clang --version 2>/dev/null | grep -oP '\d+' | head -1 || echo "17")
    LIBCXX="libc++"
else
    COMPILER="gcc"
    COMPILER_VERSION=$(gcc -dumpversion | cut -d. -f1)
    LIBCXX="libstdc++11"
fi

# Create Conan profile
mkdir -p ~/.conan2/profiles

if $USE_CLANG; then
    cat > ~/.conan2/profiles/default << EOF
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=20
compiler.libcxx=libc++
compiler.version=${COMPILER_VERSION}
os=Linux
EOF
else
    cat > ~/.conan2/profiles/default << EOF
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=20
compiler.libcxx=libstdc++11
compiler.version=${COMPILER_VERSION}
os=Linux
EOF
fi

echo "✓ Created Conan profile (${COMPILER} ${COMPILER_VERSION})"

echo ""
echo "========================================"
echo " Setup Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo "  ./scripts/build-linux.sh"
echo ""
echo "Or manually:"
echo "  mkdir -p build && cd build"
echo "  conan install .. --output-folder=. --build=missing"
echo "  cmake .. -G 'Unix Makefiles' \\"
echo "    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \\"
echo "    -DBUILD_TESTS=ON"
echo "  make -j\$(nproc)"
