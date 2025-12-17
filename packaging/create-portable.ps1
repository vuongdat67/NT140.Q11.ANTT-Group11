#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Create portable ZIP package for FileVault

.DESCRIPTION
    Creates a portable ZIP archive containing FileVault executable and documentation.
    No installation required - just extract and use.

.PARAMETER OutputDir
    Output directory for the package

.PARAMETER Version
    Version string (default: 1.0.0)

.EXAMPLE
    .\create-portable.ps1 -OutputDir .\dist -Version 1.0.0
#>

param(
    [Parameter(Mandatory=$false)]
    [string]$OutputDir = ".\dist",
    
    [Parameter(Mandatory=$false)]
    [string]$Version = "1.0.0"
)

$ErrorActionPreference = "Stop"

$AppName = "FileVault"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

# Detect build directory
$BuildDirs = @(
    "$ProjectRoot\build\build\Release\generators\build\Release\bin\release",
    "$ProjectRoot\build\build\Release\bin\release",
    "$ProjectRoot\build\Release\bin",
    "$ProjectRoot\build\bin\Release"
)

$BuildDir = $null
foreach ($dir in $BuildDirs) {
    if (Test-Path "$dir\filevault.exe") {
        $BuildDir = $dir
        break
    }
}

if (-not $BuildDir) {
    Write-Host "❌ Error: FileVault executable not found!" -ForegroundColor Red
    Write-Host "   Please build the project first using: .\scripts\build-msvc.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " FileVault Portable Package Creator" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor Green
Write-Host "Build Dir: $BuildDir" -ForegroundColor Green
Write-Host ""

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    Write-Host "✓ Created output directory: $OutputDir" -ForegroundColor Green
}

# Create staging directory
$StagingDir = Join-Path $OutputDir "FileVault-$Version-win64"
if (Test-Path $StagingDir) {
    Remove-Item -Recurse -Force $StagingDir
}
New-Item -ItemType Directory -Path $StagingDir -Force | Out-Null

Write-Host "`nPackaging files..." -ForegroundColor Cyan

# Copy executable
$ExePath = Join-Path $BuildDir "filevault.exe"
Copy-Item $ExePath $StagingDir
Write-Host "  ✓ filevault.exe" -ForegroundColor Green

# Copy any DLLs if they exist
$dllFiles = Get-ChildItem -Path $BuildDir -Filter "*.dll" -ErrorAction SilentlyContinue
foreach ($dll in $dllFiles) {
    Copy-Item $dll.FullName $StagingDir
    Write-Host "  ✓ $($dll.Name)" -ForegroundColor Green
}

# Copy documentation
$docs = @("README.md", "LICENSE")
foreach ($doc in $docs) {
    $docPath = Join-Path $ProjectRoot $doc
    if (Test-Path $docPath) {
        Copy-Item $docPath $StagingDir
        Write-Host "  ✓ $doc" -ForegroundColor Green
    }
}

# Create quick start guide
$quickStart = @"
FileVault $Version - Quick Start Guide
=======================================

This is a portable version of FileVault. No installation required!

USAGE:
------
Open a command prompt in this folder and run:

  .\filevault.exe --help          Show help
  .\filevault.exe list            List all algorithms
  .\filevault.exe encrypt file.txt    Encrypt a file
  .\filevault.exe decrypt file.enc    Decrypt a file
  .\filevault.exe benchmark       Run benchmarks

ADD TO PATH (Optional):
-----------------------
To use filevault from anywhere, add this folder to your system PATH:

1. Press Win + X and select "System"
2. Click "Advanced system settings"
3. Click "Environment Variables"
4. Under "System variables", find PATH and click Edit
5. Add the path to this folder

EXAMPLES:
---------
# Encrypt with AES-256-GCM (recommended)
.\filevault.exe encrypt document.pdf -a aes-256-gcm

# Encrypt with Post-Quantum Cryptography
.\filevault.exe keygen -a kyber-1024 -o quantum-key
.\filevault.exe encrypt secret.txt -a kyber-1024-hybrid

# Calculate file hash
.\filevault.exe hash important-file.zip

For more information, visit:
https://github.com/vuongdat67/NT140.Q11.ANTT-Group11
"@

$quickStartPath = Join-Path $StagingDir "QUICK-START.txt"
Set-Content -Path $quickStartPath -Value $quickStart -Encoding UTF8
Write-Host "  ✓ QUICK-START.txt" -ForegroundColor Green

# Create ZIP archive
$ZipPath = Join-Path $OutputDir "FileVault-$Version-win64-portable.zip"
if (Test-Path $ZipPath) {
    Remove-Item -Force $ZipPath
}

Write-Host "`nCreating ZIP archive..." -ForegroundColor Cyan
Compress-Archive -Path $StagingDir -DestinationPath $ZipPath -CompressionLevel Optimal

# Get file sizes
$zipSize = (Get-Item $ZipPath).Length
$exeSize = (Get-Item $ExePath).Length

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "✅ Portable package created!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Package: $ZipPath" -ForegroundColor White
Write-Host "Size: $([math]::Round($zipSize / 1MB, 2)) MB" -ForegroundColor White
Write-Host "Executable: $([math]::Round($exeSize / 1MB, 2)) MB" -ForegroundColor White
Write-Host ""
Write-Host "Contents:" -ForegroundColor Cyan
Get-ChildItem $StagingDir | ForEach-Object {
    $size = if ($_.PSIsContainer) { "DIR" } else { "$([math]::Round($_.Length / 1KB, 1)) KB" }
    Write-Host "  $($_.Name.PadRight(30)) $size" -ForegroundColor Gray
}

# Cleanup staging directory
Remove-Item -Recurse -Force $StagingDir

Write-Host "`nTo distribute, share: $ZipPath" -ForegroundColor Green
