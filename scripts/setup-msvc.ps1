#!/usr/bin/env pwsh
# FileVault - MSVC Setup Script for Windows
# Usage: .\scripts\setup-msvc.ps1

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " FileVault - MSVC Setup" -ForegroundColor Cyan  
Write-Host "========================================" -ForegroundColor Cyan

# Check for Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
    Write-Host "❌ Visual Studio not found!" -ForegroundColor Red
    Write-Host "Please install Visual Studio 2022 or later with C++ workload" -ForegroundColor Yellow
    exit 1
}

$vsPath = & $vsWhere -latest -property installationPath
$vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvarsall)) {
    Write-Host "❌ vcvarsall.bat not found!" -ForegroundColor Red
    exit 1
}

Write-Host "✓ Found Visual Studio at: $vsPath" -ForegroundColor Green

# Check for Ninja
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    Write-Host "Installing Ninja..." -ForegroundColor Yellow
    if (Get-Command choco -ErrorAction SilentlyContinue) {
        choco install ninja -y
    } else {
        Write-Host "❌ Please install Ninja: choco install ninja" -ForegroundColor Red
        exit 1
    }
}
Write-Host "✓ Ninja installed" -ForegroundColor Green

# Check for Python & Conan
if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Host "❌ Python not found! Please install Python 3.10+" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Python installed" -ForegroundColor Green

# Install/Update Conan
Write-Host "Installing/Updating Conan..." -ForegroundColor Yellow
python -m pip install --upgrade conan
Write-Host "✓ Conan installed" -ForegroundColor Green

# Detect MSVC version
Write-Host "`nDetecting MSVC version..." -ForegroundColor Yellow
$msvcVersion = cmd /c "`"$vcvarsall`" x64 >nul 2>&1 && cl 2>&1" | Select-String "Version (\d+\.\d+)" | ForEach-Object { $_.Matches[0].Groups[1].Value }

$conanVersion = switch -Regex ($msvcVersion) {
    "19\.5\d" { "195" }  # VS 2026
    "19\.4\d" { "194" }  # VS 2022 (late)
    "19\.3\d" { "193" }  # VS 2022 (early)
    "19\.2\d" { "192" }  # VS 2019
    default { "194" }
}

Write-Host "✓ MSVC version: $msvcVersion -> Conan version: $conanVersion" -ForegroundColor Green

# Create Conan profile
$profileDir = "$env:USERPROFILE\.conan2\profiles"
New-Item -ItemType Directory -Force -Path $profileDir | Out-Null

$profileContent = @"
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=20
compiler.runtime=dynamic
compiler.version=$conanVersion
os=Windows

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
"@

$profilePath = Join-Path $profileDir "msvc"
$profileContent | Out-File -FilePath $profilePath -Encoding utf8NoBOM
Write-Host "✓ Created Conan profile: $profilePath" -ForegroundColor Green

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " Setup Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

Write-Host @"

Next steps:
  1. Open Developer Command Prompt for VS
  2. Navigate to project: cd $((Get-Location).Path)
  3. Run: .\scripts\build-msvc.ps1

Or manually:
  mkdir build && cd build
  conan install .. --output-folder=. --build=missing -pr msvc
  cmake --preset conan-release -DBUILD_TESTS=ON
  cmake --build build/Release --parallel

"@ -ForegroundColor White
