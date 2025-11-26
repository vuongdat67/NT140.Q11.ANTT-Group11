#!/bin/bash
# FileVault - MinGW/MSYS2 Build Script
# Usage: ./scripts/build-mingw.sh [-c|--clean] [-t|--test] [-d|--debug]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

CLEAN=false
TEST=false
BUILD_TYPE="Release"

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean) CLEAN=true; shift ;;
        -t|--test) TEST=true; shift ;;
        -d|--debug) BUILD_TYPE="Debug"; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

echo "========================================"
echo " FileVault - MinGW Build"
echo "========================================"
echo "Build Type: $BUILD_TYPE"

# Check if in MSYS2
if [[ -z "$MSYSTEM" ]]; then
    echo "❌ Run this script in MSYS2 UCRT64 terminal"
    exit 1
fi

# Clean if requested
if $CLEAN; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build && cd build

# Install dependencies
echo "Installing dependencies..."
conan install .. --output-folder=. --build=missing

# Find toolchain
TOOLCHAIN=$(find . -name "conan_toolchain.cmake" -type f | head -1)
if [[ -z "$TOOLCHAIN" ]]; then
    echo "❌ Toolchain not found!"
    exit 1
fi
echo "Using toolchain: $TOOLCHAIN"

# Configure
echo "Configuring..."
cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DBUILD_TESTS=ON

# Build
echo "Building..."
ninja -j$(nproc)

# Test if requested
if $TEST; then
    echo "Running tests..."
    ctest --output-on-failure -j$(nproc)
fi

echo ""
echo "========================================"
echo " Build Successful!"
echo "========================================"
echo "Binary: build/bin/release/filevault.exe"
