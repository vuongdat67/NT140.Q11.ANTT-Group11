#!/bin/bash
# macOS Packaging Script for FileVault
# Creates DMG installer and optional Homebrew formula

set -e

APP_NAME="FileVault"
APP_VERSION="1.0.0"
BUNDLE_ID="com.nt140.filevault"
MAINTAINER="NT140.Q11.ANTT-Group15"
DESCRIPTION="Professional cross-platform file encryption CLI tool"
HOMEPAGE="https://github.com/vuongdat67/NT140.Q11.ANTT-Group15"
BUILD_DIR="./build/build/Release/bin"
OUTPUT_DIR="./dist"

echo "========================================"
echo " FileVault macOS Package Builder"
echo "========================================"
echo "Version: $APP_VERSION"
echo ""

# Check if executable exists
if [ ! -f "$BUILD_DIR/filevault" ]; then
    echo "❌ Error: FileVault executable not found at: $BUILD_DIR/filevault"
    echo "   Please build the project first"
    exit 1
fi

echo "✓ Found FileVault executable"

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Function to create DMG
create_dmg() {
    echo ""
    echo "Creating DMG installer..."
    
    DMG_DIR="$OUTPUT_DIR/dmg"
    STAGING_DIR="$DMG_DIR/staging"
    
    mkdir -p "$STAGING_DIR"
    
    # Copy executable
    cp "$BUILD_DIR/filevault" "$STAGING_DIR/"
    chmod 755 "$STAGING_DIR/filevault"
    
    # Create README
    cat > "$STAGING_DIR/README.txt" << EOF
FileVault v$APP_VERSION
========================

Installation Instructions:
--------------------------
1. Copy 'filevault' to /usr/local/bin:
   
   sudo cp filevault /usr/local/bin/
   
2. Make sure /usr/local/bin is in your PATH:
   
   echo 'export PATH="/usr/local/bin:\$PATH"' >> ~/.zshrc
   source ~/.zshrc

3. Verify installation:
   
   filevault --version

Usage:
------
Run 'filevault --help' for usage information.

For more information, visit:
$HOMEPAGE

Uninstallation:
---------------
To remove FileVault, simply delete the binary:
   sudo rm /usr/local/bin/filevault
EOF

    # Create install script
    cat > "$STAGING_DIR/install.sh" << 'EOF'
#!/bin/bash
set -e

echo "Installing FileVault..."

# Check for sudo
if [ "$EUID" -ne 0 ]; then 
    echo "This script needs sudo privileges"
    exec sudo "$0" "$@"
fi

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Copy to /usr/local/bin
cp "$SCRIPT_DIR/filevault" /usr/local/bin/
chmod 755 /usr/local/bin/filevault

echo "✓ FileVault installed to /usr/local/bin/filevault"
echo ""
echo "Run 'filevault --help' to get started"

exit 0
EOF
    chmod 755 "$STAGING_DIR/install.sh"
    
    # Create uninstall script
    cat > "$STAGING_DIR/uninstall.sh" << 'EOF'
#!/bin/bash
set -e

echo "Uninstalling FileVault..."

# Check for sudo
if [ "$EUID" -ne 0 ]; then 
    echo "This script needs sudo privileges"
    exec sudo "$0" "$@"
fi

# Remove from /usr/local/bin
rm -f /usr/local/bin/filevault

echo "✓ FileVault uninstalled"

exit 0
EOF
    chmod 755 "$STAGING_DIR/uninstall.sh"
    
    # Create DMG
    DMG_FILE="$OUTPUT_DIR/$APP_NAME-$APP_VERSION-macOS.dmg"
    
    # Use hdiutil to create DMG
    hdiutil create -volname "$APP_NAME" \
        -srcfolder "$STAGING_DIR" \
        -ov -format UDZO \
        "$DMG_FILE"
    
    echo "✓ DMG created: $DMG_FILE"
    
    # Clean up
    rm -rf "$DMG_DIR"
}

# Function to create Homebrew formula
create_homebrew_formula() {
    echo ""
    echo "Creating Homebrew formula..."
    
    FORMULA_FILE="$OUTPUT_DIR/filevault.rb"
    
    # Calculate SHA256 (assuming tarball exists)
    TARBALL="$OUTPUT_DIR/filevault-$APP_VERSION.tar.gz"
    
    if [ ! -f "$TARBALL" ]; then
        echo "Creating source tarball..."
        tar czf "$TARBALL" \
            --exclude='.git' \
            --exclude='build' \
            --exclude='dist' \
            --transform "s,^,filevault-$APP_VERSION/," \
            $(git ls-files 2>/dev/null || find . -type f -not -path '*/\.*' -not -path '*/build/*' -not -path '*/dist/*')
    fi
    
    SHA256=$(shasum -a 256 "$TARBALL" | awk '{print $1}')
    
    cat > "$FORMULA_FILE" << EOF
class Filevault < Formula
  desc "$DESCRIPTION"
  homepage "$HOMEPAGE"
  url "$HOMEPAGE/releases/download/v$APP_VERSION/filevault-$APP_VERSION.tar.gz"
  sha256 "$SHA256"
  license "MIT"
  
  depends_on "cmake" => :build
  depends_on "conan" => :build
  depends_on "ninja" => :build

  def install
    system "conan", "install", ".", "--output-folder=build", "--build=missing"
    system "cmake", "--preset", "conan-release"
    system "cmake", "--build", "--preset", "conan-release"
    
    bin.install "build/build/Release/bin/filevault"
  end

  test do
    assert_match version.to_s, shell_output("#{bin}/filevault --version")
  end
end
EOF

    echo "✓ Homebrew formula created: $FORMULA_FILE"
    echo ""
    echo "To use this formula:"
    echo "  1. Create a tap repository on GitHub (e.g., homebrew-filevault)"
    echo "  2. Copy this formula to Formula/ directory"
    echo "  3. Users can install with: brew install yourusername/filevault/filevault"
}

# Function to create PKG installer
create_pkg() {
    echo ""
    echo "Creating PKG installer..."
    
    PKG_DIR="$OUTPUT_DIR/pkg"
    PKG_ROOT="$PKG_DIR/root"
    PKG_SCRIPTS="$PKG_DIR/scripts"
    
    mkdir -p "$PKG_ROOT/usr/local/bin"
    mkdir -p "$PKG_SCRIPTS"
    
    # Copy executable
    cp "$BUILD_DIR/filevault" "$PKG_ROOT/usr/local/bin/"
    chmod 755 "$PKG_ROOT/usr/local/bin/filevault"
    
    # Create postinstall script
    cat > "$PKG_SCRIPTS/postinstall" << 'EOF'
#!/bin/bash
set -e

# Ensure /usr/local/bin is in PATH
if ! grep -q '/usr/local/bin' /etc/paths; then
    echo '/usr/local/bin' >> /etc/paths
fi

echo "FileVault installed successfully!"
echo "Run 'filevault --help' to get started"

exit 0
EOF
    chmod 755 "$PKG_SCRIPTS/postinstall"
    
    # Build PKG
    PKG_FILE="$OUTPUT_DIR/$APP_NAME-$APP_VERSION-macOS.pkg"
    
    pkgbuild --root "$PKG_ROOT" \
        --identifier "$BUNDLE_ID" \
        --version "$APP_VERSION" \
        --scripts "$PKG_SCRIPTS" \
        --install-location "/" \
        "$PKG_FILE"
    
    echo "✓ PKG installer created: $PKG_FILE"
    
    # Create product archive (signed package)
    # Uncomment if you have a Developer ID certificate
    # SIGNED_PKG="$OUTPUT_DIR/$APP_NAME-$APP_VERSION-macOS-signed.pkg"
    # productbuild --sign "Developer ID Installer: Your Name" \
    #     --package "$PKG_FILE" \
    #     "$SIGNED_PKG"
    
    # Clean up
    rm -rf "$PKG_DIR"
}

# Main execution
echo "Select package type:"
echo "  1) DMG (Disk Image)"
echo "  2) PKG (Package Installer)"
echo "  3) Homebrew Formula"
echo "  4) All"
read -p "Enter choice [1-4]: " choice

case $choice in
    1)
        create_dmg
        ;;
    2)
        create_pkg
        ;;
    3)
        create_homebrew_formula
        ;;
    4)
        create_dmg
        create_pkg
        create_homebrew_formula
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac

echo ""
echo "========================================"
echo "✅ Packaging completed!"
echo "========================================"
echo "Output directory: $OUTPUT_DIR"
ls -lh "$OUTPUT_DIR"
