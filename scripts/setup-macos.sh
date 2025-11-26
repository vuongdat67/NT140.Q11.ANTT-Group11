#!/bin/bash
# FileVault - macOS Setup Script
# Usage: ./scripts/setup-macos.sh

set -e

echo "========================================"
echo " FileVault - macOS Setup"
echo "========================================"

# Check for Xcode Command Line Tools
if ! xcode-select -p &> /dev/null; then
    echo "Installing Xcode Command Line Tools..."
    xcode-select --install
    echo "Please run this script again after installation completes."
    exit 0
fi
echo "✓ Xcode Command Line Tools installed"

# Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi
echo "✓ Homebrew installed"

# Install dependencies
echo "Installing dependencies..."
brew install cmake ninja python

# Install Conan
pip3 install conan
echo "✓ Conan installed"

# Get compiler version
CLANG_VERSION=$(clang --version | grep -oE '\d+\.\d+' | head -1 | cut -d. -f1)

# Detect architecture
ARCH=$(uname -m)
if [[ "$ARCH" == "arm64" ]]; then
    CONAN_ARCH="armv8"
else
    CONAN_ARCH="x86_64"
fi

# Create Conan profile
mkdir -p ~/.conan2/profiles
cat > ~/.conan2/profiles/default << EOF
[settings]
arch=${CONAN_ARCH}
build_type=Release
compiler=apple-clang
compiler.cppstd=20
compiler.libcxx=libc++
compiler.version=${CLANG_VERSION}
os=Macos
EOF

echo "✓ Created Conan profile (Apple Clang ${CLANG_VERSION}, ${ARCH})"

echo ""
echo "========================================"
echo " Setup Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo "  ./scripts/build-macos.sh"
echo ""
echo "Or manually:"
echo "  mkdir -p build && cd build"
echo "  conan install .. --output-folder=. --build=missing"
echo "  cmake .. -G 'Unix Makefiles' \\"
echo "    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \\"
echo "    -DBUILD_TESTS=ON"
echo "  make -j\$(sysctl -n hw.ncpu)"
