# FileVault Setup Script for Windows (PowerShell)
# Uses Conan to download pre-compiled packages

#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

# Colors
function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Write-Warn {
    param([string]$Message)
    Write-Host "[WARN] $Message" -ForegroundColor Yellow
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# Check if running as Administrator
function Test-Administrator {
    $user = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($user)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Install winget packages
function Install-WingetPackage {
    param([string]$PackageId)
    
    if (winget list --id $PackageId -e) {
        Write-Info "$PackageId already installed"
    } else {
        Write-Info "Installing $PackageId..."
        winget install --id $PackageId -e --silent --accept-source-agreements --accept-package-agreements
    }
}

# Install dependencies
function Install-Dependencies {
    Write-Info "Installing dependencies..."
    
    # Install Visual Studio Build Tools 2022
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        Write-Info "Installing Visual Studio Build Tools..."
        Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools"
    }
    
    # Install other tools
    Install-WingetPackage "Kitware.CMake"
    Install-WingetPackage "Git.Git"
    Install-WingetPackage "Python.Python.3.12"
    
    # Refresh environment
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    
    Write-Info "Dependencies installed successfully"
}

# Install Conan 2.x
function Install-Conan {
    Write-Info "Checking Conan installation..."
    
    if (Get-Command conan -ErrorAction SilentlyContinue) {
        $conanVersion = conan --version
        Write-Info "Conan already installed: $conanVersion"
    } else {
        Write-Info "Installing Conan 2.x..."
        python -m pip install --upgrade pip
        python -m pip install "conan>=2.0"
        
        # Add Python Scripts to PATH
        $pythonScripts = "$env:USERPROFILE\AppData\Local\Programs\Python\Python312\Scripts"
        $env:Path += ";$pythonScripts"
        [Environment]::SetEnvironmentVariable("Path", $env:Path, "User")
    }
    
    # Detect compiler profile
    Write-Info "Detecting Conan profile..."
    conan profile detect --force
    
    # Show profile
    Write-Info "Conan profile:"
    conan profile show default
}

# Install dependencies via Conan
function Install-ConanDependencies {
    Write-Info "Installing dependencies via Conan..."
    
    Set-Location $ProjectRoot
    
    # Create build directory
    if (-not (Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" | Out-Null
    }
    Set-Location "build"
    
    Write-Info "Downloading pre-compiled packages from Conan Center..."
    Write-Warn "First time will take 5-10 minutes to download packages"
    
    # Install dependencies (Conan downloads pre-built binaries!)
    conan install .. --build=missing -s build_type=Release
    
    Write-Info "All dependencies installed!"
}

# Build project
function Build-Project {
    Write-Info "Building FileVault..."
    
    Set-Location "$ProjectRoot\build"
    
    # Detect generator
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        $generator = "Ninja"
        Write-Info "Using Ninja generator (faster)"
    } else {
        $generator = "Visual Studio 17 2022"
        Write-Info "Using Visual Studio generator"
    }
    
    # Configure with CMake
    Write-Info "Configuring CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release `
             -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake `
             -G $generator
    
    # Build
    Write-Info "Compiling..."
    cmake --build . --config Release --parallel
    
    Write-Info "Build completed successfully!"
}

# Run tests
function Run-Tests {
    Write-Info "Running tests..."
    
    Set-Location "$ProjectRoot\build"
    ctest -C Release --output-on-failure
    
    Write-Info "All tests passed!"
}

# Install binary
function Install-Binary {
    Write-Info "Installing FileVault..."
    
    $installDir = "$env:ProgramFiles\FileVault"
    
    if (-not (Test-Path $installDir)) {
        New-Item -ItemType Directory -Path $installDir | Out-Null
    }
    
    Copy-Item "$ProjectRoot\build\Release\filevault.exe" -Destination $installDir
    
    # Add to PATH
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    if ($currentPath -notlike "*$installDir*") {
        [Environment]::SetEnvironmentVariable("Path", "$currentPath;$installDir", "Machine")
    }
    
    Write-Info "FileVault installed to $installDir"
    Write-Info "Added to system PATH"
}

# Verify installation
function Verify-Installation {
    Write-Info "Verifying installation..."
    
    # Check Conan
    $conanVersion = conan --version
    Write-Info "✓ Conan: $conanVersion"
    
    # Check CMake
    $cmakeVersion = cmake --version | Select-Object -First 1
    Write-Info "✓ CMake: $cmakeVersion"
    
    # Check Python
    $pythonVersion = python --version
    Write-Info "✓ Python: $pythonVersion"
    
    # Check compiler
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Info "✓ MSVC compiler found"
    } else {
        Write-Warn "MSVC compiler not found in PATH"
    }
}

# Main setup function
function Main {
    Write-Host ""
    Write-Info "╔════════════════════════════════════════════════╗"
    Write-Info "║   FileVault Setup Script for Windows 11       ║"
    Write-Info "╚════════════════════════════════════════════════╝"
    Write-Host ""
    Write-Info "Project root: $ProjectRoot"
    Write-Host ""
    
    if (-not (Test-Administrator)) {
        Write-Error-Custom "This script must be run as Administrator!"
        Write-Warn "Right-click PowerShell and 'Run as Administrator'"
        exit 1
    }
    
    # Step 1: Install tools
    Write-Host ""
    Write-Info "=== Step 1/5: Installing Development Tools ==="
    Install-Dependencies
    
    # Step 2: Install Conan
    Write-Host ""
    Write-Info "=== Step 2/5: Installing Conan Package Manager ==="
    Install-Conan
    
    # Step 3: Install dependencies
    Write-Host ""
    Write-Info "=== Step 3/5: Downloading Pre-compiled Dependencies ==="
    Install-ConanDependencies
    
    # Step 4: Build
    Write-Host ""
    Write-Info "=== Step 4/5: Building FileVault ==="
    Build-Project
    
    # Verify
    Write-Host ""
    Write-Info "=== Verification ==="
    Verify-Installation
    
    # Ask if user wants to run tests
    Write-Host ""
    $runTests = Read-Host "Do you want to run tests? (y/n)"
    if ($runTests -eq 'y' -or $runTests -eq 'Y') {
        Run-Tests
    }
    
    # Ask if user wants to install
    Write-Host ""
    $installSystem = Read-Host "Do you want to install FileVault system-wide? (y/n)"
    if ($installSystem -eq 'y' -or $installSystem -eq 'Y') {
        Install-Binary
    }
    
    # Final message
    Write-Host ""
    Write-Info "╔════════════════════════════════════════════════╗"
    Write-Info "║          Setup Completed Successfully!        ║"
    Write-Info "╚════════════════════════════════════════════════╝"
    Write-Host ""
    
    Write-Info "Next steps:"
    Write-Info "1. cd $ProjectRoot\build"
    Write-Info "2. .\Release\filevault.exe --help"
    Write-Host ""
    
    if ($installSystem -eq 'y' -or $installSystem -eq 'Y') {
        Write-Info "Or simply run: filevault --help"
        Write-Warn "⚠️  You may need to restart your terminal for PATH changes"
    }
    
    Write-Host ""
    Write-Info "Documentation: .github/WINDOWS_SETUP.md"
    Write-Info "Troubleshooting: .github/TROUBLESHOOTING.md"
    Write-Host ""
}

# Run main
Main