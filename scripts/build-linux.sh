#!/bin/bash
# FileVault - Linux Build Script
# Usage: ./scripts/build-linux.sh [-c|--clean] [-t|--test] [-d|--debug] [--clang]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

CLEAN=false
TEST=false
BUILD_TYPE="Release"
USE_CLANG=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean) CLEAN=true; shift ;;
        -t|--test) TEST=true; shift ;;
        -d|--debug) BUILD_TYPE="Debug"; shift ;;
        --clang) USE_CLANG=true; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

echo "========================================"
echo " FileVault - Linux Build"
echo "========================================"
echo "Build Type: $BUILD_TYPE"
echo "Compiler: $(if $USE_CLANG; then echo 'Clang'; else echo 'GCC'; fi)"

# Clean if requested
if $CLEAN; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build && cd build

# Install dependencies
echo "Installing dependencies..."
if $USE_CLANG; then
    conan install .. --output-folder=. --build=missing -pr clang 2>/dev/null || \
    conan install .. --output-folder=. --build=missing
else
    conan install .. --output-folder=. --build=missing
fi

# Find toolchain
TOOLCHAIN=$(find . -name "conan_toolchain.cmake" -type f | head -1)
if [[ -z "$TOOLCHAIN" ]]; then
    echo "❌ Toolchain not found!"
    exit 1
fi
echo "Using toolchain: $TOOLCHAIN"

# Configure
echo "Configuring..."
CMAKE_ARGS=(
    -G "Unix Makefiles"
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
    -DBUILD_TESTS=ON
)

if $USE_CLANG; then
    CMAKE_ARGS+=(
        -DCMAKE_C_COMPILER=clang
        -DCMAKE_CXX_COMPILER=clang++
    )
fi

cmake .. "${CMAKE_ARGS[@]}"

# Build
echo "Building..."
make -j$(nproc)

# Test if requested
if $TEST; then
    echo "Running tests..."
    ctest --output-on-failure -j$(nproc)
fi

echo ""
echo "========================================"
echo " Build Successful!"
echo "========================================"
echo "Binary: build/bin/release/filevault"
