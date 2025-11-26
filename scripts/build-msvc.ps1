#!/usr/bin/env pwsh
# FileVault - MSVC Build Script for Windows
# Usage: .\scripts\build-msvc.ps1 [-Clean] [-Test] [-Release] [-Debug]

param(
    [switch]$Clean,
    [switch]$Test,
    [switch]$Release = $true,
    [switch]$Debug
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $ProjectRoot

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " FileVault - MSVC Build" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Find vcvarsall.bat
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vsWhere -latest -property installationPath
$vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvarsall)) {
    Write-Host "❌ Visual Studio not found!" -ForegroundColor Red
    exit 1
}

$BuildType = if ($Debug) { "Debug" } else { "Release" }
Write-Host "Build Type: $BuildType" -ForegroundColor Yellow

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
}

# Create build directory
New-Item -ItemType Directory -Force -Path build | Out-Null
Set-Location build

# Run in VS environment
$commands = @(
    "conan install .. --output-folder=. --build=missing -pr msvc",
    "cmake --preset conan-$($BuildType.ToLower()) -DBUILD_TESTS=ON",
    "cmake --build build/$BuildType --parallel $env:NUMBER_OF_PROCESSORS"
)

if ($Test) {
    $commands += "ctest --test-dir build/$BuildType --output-on-failure -j $env:NUMBER_OF_PROCESSORS"
}

$script = $commands -join " && "

Write-Host "`nRunning build commands..." -ForegroundColor Yellow
cmd /c "`"$vcvarsall`" x64 && $script"

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host " Build Successful!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Binary: build\bin\$($BuildType.ToLower())\filevault.exe" -ForegroundColor White
} else {
    Write-Host "`n❌ Build Failed!" -ForegroundColor Red
    exit 1
}

Set-Location $ProjectRoot
