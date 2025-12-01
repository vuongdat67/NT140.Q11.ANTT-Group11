# FileVault Packaging

This directory contains scripts to create installers for different platforms.

## 📦 Supported Platforms

### Windows
- **MSI Installer** (WiX Toolset)
- **NSIS Installer** (Nullsoft Scriptable Install System)

### Linux
- **DEB Package** (Debian/Ubuntu)
- **RPM Package** (RedHat/Fedora/SUSE)
- **AppImage** (Universal Linux package)

### macOS
- **DMG** (Disk Image with drag-and-drop)
- **PKG** (Package Installer)
- **Homebrew Formula**

## 🚀 Usage

### Windows

#### Prerequisites
- **For MSI**: Install [WiX Toolset](https://wixtoolset.org/releases/)
- **For NSIS**: Install [NSIS](https://nsis.sourceforge.io/Download)

```powershell
# Build the project first
.\scripts\build-msvc.ps1

# Create NSIS installer (recommended)
.\packaging\windows-installer.ps1 -Type nsis

# Or create MSI installer
.\packaging\windows-installer.ps1 -Type msi

# Output: .\dist\FileVault-1.0.0-x64-setup.exe
```

### Linux

#### Prerequisites
```bash
# For DEB packages
sudo apt install dpkg-dev

# For RPM packages  
sudo apt install rpm  # or: sudo yum install rpm-build

# For AppImage
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
sudo mv appimagetool-x86_64.AppImage /usr/local/bin/appimagetool
```

```bash
# Build the project first
./scripts/build-linux.sh

# Make script executable
chmod +x ./packaging/linux-package.sh

# Run packaging script
./packaging/linux-package.sh

# Select package type when prompted:
# 1) DEB - for Debian/Ubuntu
# 2) RPM - for RedHat/Fedora
# 3) AppImage - Universal
# 4) All - Create all packages
```

#### Installing Packages

**DEB (Debian/Ubuntu):**
```bash
sudo dpkg -i dist/filevault_1.0.0_amd64.deb
# Or
sudo apt install ./dist/filevault_1.0.0_amd64.deb
```

**RPM (RedHat/Fedora):**
```bash
sudo rpm -i dist/filevault-1.0.0-1.x86_64.rpm
# Or
sudo dnf install dist/filevault-1.0.0-1.x86_64.rpm
```

**AppImage:**
```bash
chmod +x dist/FileVault-1.0.0-x86_64.AppImage
./dist/FileVault-1.0.0-x86_64.AppImage --help
```

### macOS

```bash
# Build the project first
./scripts/build-macos.sh

# Make script executable
chmod +x ./packaging/macos-package.sh

# Run packaging script
./packaging/macos-package.sh

# Select package type when prompted:
# 1) DMG - Disk image (recommended)
# 2) PKG - Package installer
# 3) Homebrew Formula
# 4) All - Create all packages
```

#### Installing Packages

**DMG:**
1. Open `dist/FileVault-1.0.0-macOS.dmg`
2. Run `install.sh` script
3. Or manually copy `filevault` to `/usr/local/bin/`

**PKG:**
```bash
sudo installer -pkg dist/FileVault-1.0.0-macOS.pkg -target /
```

**Homebrew:**
```bash
# After publishing the formula to a tap
brew install yourusername/tap/filevault
```

## 📝 Configuration

Edit the following variables in each script:

- `APP_NAME` - Application name
- `APP_VERSION` - Version number
- `PUBLISHER` / `MAINTAINER` - Your organization
- `HOMEPAGE` - Project URL
- `BUILD_DIR` - Location of built executable

## 🔐 Code Signing

### Windows
```powershell
# Sign the installer (requires certificate)
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com dist/FileVault-1.0.0-x64-setup.exe
```

### macOS
```bash
# Sign the executable (requires Apple Developer certificate)
codesign --sign "Developer ID Application: Your Name" build/build/Release/bin/filevault

# Sign and notarize the PKG
productsign --sign "Developer ID Installer: Your Name" \
    dist/FileVault-1.0.0-macOS.pkg \
    dist/FileVault-1.0.0-macOS-signed.pkg

xcrun notarytool submit dist/FileVault-1.0.0-macOS-signed.pkg \
    --apple-id your@email.com \
    --password app-specific-password \
    --team-id TEAMID
```

## 📂 Output Structure

After running packaging scripts, the `dist/` directory will contain:

```
dist/
├── FileVault-1.0.0-x64-setup.exe      # Windows NSIS installer
├── FileVault-1.0.0-x64.msi            # Windows MSI installer
├── filevault_1.0.0_amd64.deb          # Debian/Ubuntu package
├── filevault-1.0.0-1.x86_64.rpm       # RedHat/Fedora package
├── FileVault-1.0.0-x86_64.AppImage    # Universal Linux package
├── FileVault-1.0.0-macOS.dmg          # macOS disk image
├── FileVault-1.0.0-macOS.pkg          # macOS package installer
└── filevault.rb                       # Homebrew formula
```

## 🧪 Testing Installers

### Windows
```powershell
# Test NSIS installer
Start-Process dist/FileVault-1.0.0-x64-setup.exe -ArgumentList '/S' -Wait

# Verify installation
filevault --version
```

### Linux (DEB)
```bash
# Install in test environment
docker run -it --rm -v $(pwd)/dist:/dist ubuntu:22.04
apt update && apt install -y /dist/filevault_1.0.0_amd64.deb
filevault --version
```

### macOS
```bash
# Test PKG in temporary location
sudo installer -pkg dist/FileVault-1.0.0-macOS.pkg -target /tmp/test
/tmp/test/usr/local/bin/filevault --version
```

## 🚀 CI/CD Integration

### GitHub Actions Example

```yaml
name: Build Installers

on:
  release:
    types: [created]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: .\scripts\build-msvc.ps1
      - name: Create Installer
        run: .\packaging\windows-installer.ps1 -Type nsis
      - name: Upload Artifact
        uses: actions/upload-artifact@v3
        with:
          name: windows-installer
          path: dist/*.exe

  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: ./scripts/build-linux.sh
      - name: Create DEB
        run: ./packaging/linux-package.sh <<< "1"
      - name: Upload Artifact
        uses: actions/upload-artifact@v3
        with:
          name: linux-deb
          path: dist/*.deb

  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: ./scripts/build-macos.sh
      - name: Create DMG
        run: ./packaging/macos-package.sh <<< "1"
      - name: Upload Artifact
        uses: actions/upload-artifact@v3
        with:
          name: macos-dmg
          path: dist/*.dmg
```

## 📚 Additional Resources

- [WiX Toolset Documentation](https://wixtoolset.org/documentation/)
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [Debian Package Guide](https://www.debian.org/doc/manuals/maint-guide/)
- [RPM Packaging Guide](https://rpm-packaging-guide.github.io/)
- [AppImage Documentation](https://docs.appimage.org/)
- [macOS PKG Guide](https://developer.apple.com/library/archive/documentation/DeveloperTools/Reference/DistributionDefinitionRef/Chapters/Introduction.html)

## ⚠️ Notes

- Always test installers in clean environments before distribution
- Update version numbers in all scripts before releasing
- Code signing is recommended for production releases
- Consider notarization for macOS packages
- Test uninstallation procedures
