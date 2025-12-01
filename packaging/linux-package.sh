#!/bin/bash
# Linux Packaging Script for FileVault
# Creates DEB and RPM packages

set -e

APP_NAME="filevault"
APP_VERSION="1.0.0"
MAINTAINER="NT140.Q11.ANTT-Group15 <contact@example.com>"
DESCRIPTION="Professional cross-platform file encryption CLI tool"
HOMEPAGE="https://github.com/vuongdat67/NT140.Q11.ANTT-Group15"
BUILD_DIR="./build/build/Release/bin"
OUTPUT_DIR="./dist"

echo "========================================"
echo " FileVault Linux Package Builder"
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

# Function to create DEB package
create_deb() {
    echo ""
    echo "Creating DEB package..."
    
    DEB_DIR="$OUTPUT_DIR/deb"
    PACKAGE_DIR="$DEB_DIR/${APP_NAME}_${APP_VERSION}_amd64"
    
    # Create directory structure
    mkdir -p "$PACKAGE_DIR/usr/bin"
    mkdir -p "$PACKAGE_DIR/usr/share/doc/$APP_NAME"
    mkdir -p "$PACKAGE_DIR/usr/share/man/man1"
    mkdir -p "$PACKAGE_DIR/DEBIAN"
    
    # Copy executable
    cp "$BUILD_DIR/filevault" "$PACKAGE_DIR/usr/bin/"
    chmod 755 "$PACKAGE_DIR/usr/bin/filevault"
    
    # Copy documentation
    cp README.md "$PACKAGE_DIR/usr/share/doc/$APP_NAME/"
    cp LICENSE "$PACKAGE_DIR/usr/share/doc/$APP_NAME/" 2>/dev/null || echo "No LICENSE file"
    
    # Create control file
    cat > "$PACKAGE_DIR/DEBIAN/control" << EOF
Package: $APP_NAME
Version: $APP_VERSION
Section: utils
Priority: optional
Architecture: amd64
Depends: libc6 (>= 2.34), libstdc++6 (>= 12)
Maintainer: $MAINTAINER
Homepage: $HOMEPAGE
Description: $DESCRIPTION
 FileVault is a professional cross-platform file encryption tool with support for:
 - Modern AEAD ciphers (AES-GCM, ChaCha20-Poly1305)
 - Post-Quantum Cryptography (Kyber, Dilithium)
 - Asymmetric encryption (RSA, ECC)
 - Compression (ZLIB, LZMA)
 - Steganography and more
EOF

    # Create postinst script
    cat > "$PACKAGE_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

# Add to PATH if not already there
if ! grep -q "/usr/bin" /etc/environment; then
    echo "FileVault installed successfully!"
fi

exit 0
EOF
    chmod 755 "$PACKAGE_DIR/DEBIAN/postinst"
    
    # Create prerm script
    cat > "$PACKAGE_DIR/DEBIAN/prerm" << 'EOF'
#!/bin/bash
set -e
exit 0
EOF
    chmod 755 "$PACKAGE_DIR/DEBIAN/prerm"
    
    # Build DEB package
    dpkg-deb --build "$PACKAGE_DIR"
    
    DEB_FILE="$OUTPUT_DIR/${APP_NAME}_${APP_VERSION}_amd64.deb"
    mv "$PACKAGE_DIR.deb" "$DEB_FILE"
    
    echo "✓ DEB package created: $DEB_FILE"
    
    # Verify package
    dpkg-deb --info "$DEB_FILE"
    
    # Clean up
    rm -rf "$DEB_DIR"
}

# Function to create RPM package
create_rpm() {
    echo ""
    echo "Creating RPM package..."
    
    # Check for rpmbuild
    if ! command -v rpmbuild &> /dev/null; then
        echo "⚠️  rpmbuild not found. Install rpm-build package."
        echo "   Skipping RPM creation."
        return
    fi
    
    RPM_DIR="$OUTPUT_DIR/rpm"
    mkdir -p "$RPM_DIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    
    # Create tarball for source
    TARBALL_DIR="${APP_NAME}-${APP_VERSION}"
    mkdir -p "$RPM_DIR/SOURCES/$TARBALL_DIR/usr/bin"
    cp "$BUILD_DIR/filevault" "$RPM_DIR/SOURCES/$TARBALL_DIR/usr/bin/"
    
    (cd "$RPM_DIR/SOURCES" && tar czf "${APP_NAME}-${APP_VERSION}.tar.gz" "$TARBALL_DIR")
    rm -rf "$RPM_DIR/SOURCES/$TARBALL_DIR"
    
    # Create spec file
    cat > "$RPM_DIR/SPECS/$APP_NAME.spec" << EOF
Name:           $APP_NAME
Version:        $APP_VERSION
Release:        1%{?dist}
Summary:        $DESCRIPTION

License:        MIT
URL:            $HOMEPAGE
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
Requires:       glibc, libstdc++

%description
FileVault is a professional cross-platform file encryption tool with support for
modern AEAD ciphers, Post-Quantum Cryptography, asymmetric encryption, compression,
steganography and more.

%prep
%setup -q

%install
mkdir -p %{buildroot}/usr/bin
cp usr/bin/filevault %{buildroot}/usr/bin/
chmod 755 %{buildroot}/usr/bin/filevault

%files
/usr/bin/filevault

%changelog
* $(date +"%a %b %d %Y") $MAINTAINER - $APP_VERSION-1
- Initial RPM release
EOF

    # Build RPM
    rpmbuild --define "_topdir $RPM_DIR" -ba "$RPM_DIR/SPECS/$APP_NAME.spec"
    
    # Find and move RPM
    RPM_FILE=$(find "$RPM_DIR/RPMS" -name "*.rpm" | head -n1)
    if [ -n "$RPM_FILE" ]; then
        mv "$RPM_FILE" "$OUTPUT_DIR/"
        RPM_BASENAME=$(basename "$RPM_FILE")
        echo "✓ RPM package created: $OUTPUT_DIR/$RPM_BASENAME"
    fi
    
    # Clean up
    rm -rf "$RPM_DIR"
}

# Function to create AppImage
create_appimage() {
    echo ""
    echo "Creating AppImage..."
    
    # Check for appimagetool
    if ! command -v appimagetool &> /dev/null; then
        echo "⚠️  appimagetool not found."
        echo "   Download from: https://github.com/AppImage/AppImageKit/releases"
        echo "   Skipping AppImage creation."
        return
    fi
    
    APPDIR="$OUTPUT_DIR/FileVault.AppDir"
    mkdir -p "$APPDIR/usr/bin"
    mkdir -p "$APPDIR/usr/share/applications"
    mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
    
    # Copy executable
    cp "$BUILD_DIR/filevault" "$APPDIR/usr/bin/"
    chmod 755 "$APPDIR/usr/bin/filevault"
    
    # Create desktop entry
    cat > "$APPDIR/usr/share/applications/$APP_NAME.desktop" << EOF
[Desktop Entry]
Type=Application
Name=FileVault
Comment=$DESCRIPTION
Exec=filevault
Icon=filevault
Categories=Utility;Security;
Terminal=true
EOF

    # Create AppRun
    cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin:${PATH}"
exec "${HERE}/usr/bin/filevault" "$@"
EOF
    chmod 755 "$APPDIR/AppRun"
    
    # Create icon (placeholder if not exists)
    if [ ! -f "$APPDIR/usr/share/icons/hicolor/256x256/apps/filevault.png" ]; then
        echo "⚠️  No icon found, creating placeholder"
    fi
    
    ln -sf usr/share/applications/$APP_NAME.desktop "$APPDIR/"
    ln -sf usr/share/icons/hicolor/256x256/apps/filevault.png "$APPDIR/" 2>/dev/null || true
    
    # Build AppImage
    appimagetool "$APPDIR" "$OUTPUT_DIR/FileVault-$APP_VERSION-x86_64.AppImage"
    
    echo "✓ AppImage created: $OUTPUT_DIR/FileVault-$APP_VERSION-x86_64.AppImage"
    
    # Clean up
    rm -rf "$APPDIR"
}

# Main execution
echo "Select package type:"
echo "  1) DEB (Debian/Ubuntu)"
echo "  2) RPM (RedHat/Fedora/SUSE)"
echo "  3) AppImage (Universal)"
echo "  4) All"
read -p "Enter choice [1-4]: " choice

case $choice in
    1)
        create_deb
        ;;
    2)
        create_rpm
        ;;
    3)
        create_appimage
        ;;
    4)
        create_deb
        create_rpm
        create_appimage
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
