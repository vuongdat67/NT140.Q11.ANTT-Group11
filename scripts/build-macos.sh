#!/bin/bash
# FileVault - macOS Build Script
# Usage: ./scripts/build-macos.sh [-c|--clean] [-t|--test] [-d|--debug]

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
echo " FileVault - macOS Build"
echo "========================================"
echo "Build Type: $BUILD_TYPE"
echo "Architecture: $(uname -m)"

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
cmake .. -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_OSX_ARCHITECTURES=$(uname -m) \
    -DBUILD_TESTS=ON

# Build
echo "Building..."
make -j$(sysctl -n hw.ncpu)

# Test if requested
if $TEST; then
    echo "Running tests..."
    ctest --output-on-failure -j$(sysctl -n hw.ncpu)
fi

echo ""
echo "========================================"
echo " Build Successful!"
echo "========================================"
echo "Binary: build/bin/release/filevault"
