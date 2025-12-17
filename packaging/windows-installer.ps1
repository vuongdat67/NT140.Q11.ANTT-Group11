<#
.SYNOPSIS
    Create Windows installer for FileVault

.DESCRIPTION
    Creates MSI installer using WiX Toolset or NSIS installer
    
.PARAMETER Type
    Installer type: 'msi' (WiX) or 'nsis' (NSIS)
    
.PARAMETER OutputDir
    Output directory for installer package

.EXAMPLE
    .\windows-installer.ps1 -Type msi -OutputDir .\dist
#>

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('msi', 'nsis')]
    [string]$Type = 'nsis',
    
    [Parameter(Mandatory=$false)]
    [string]$OutputDir = ".\dist"
)

$ErrorActionPreference = "Stop"

# Configuration
$AppName = "FileVault"
$AppVersion = "1.0.0"
$Publisher = "NT140.Q11.ANTT-Group11"
$AppUrl = "https://github.com/vuongdat67/NT140.Q11.ANTT-Group11"
$BuildDir = ".\build\build\Release\bin"
$IconPath = ".\assets\icon.ico"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " FileVault Windows Installer Builder" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Type: $Type" -ForegroundColor Green
Write-Host "Version: $AppVersion" -ForegroundColor Green
Write-Host ""

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
    Write-Host "✓ Created output directory: $OutputDir" -ForegroundColor Green
}

# Check if executable exists
$ExePath = Join-Path $BuildDir "filevault.exe"
if (-not (Test-Path $ExePath)) {
    Write-Host "❌ Error: FileVault executable not found at: $ExePath" -ForegroundColor Red
    Write-Host "   Please build the project first using: .\scripts\build-msvc.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ Found FileVault executable" -ForegroundColor Green

if ($Type -eq 'msi') {
    # WiX Toolset installer
    Write-Host "`nCreating WiX MSI installer..." -ForegroundColor Cyan
    
    # Check for WiX
    $wixPath = "candle.exe"
    if (-not (Get-Command $wixPath -ErrorAction SilentlyContinue)) {
        Write-Host "❌ Error: WiX Toolset not found!" -ForegroundColor Red
        Write-Host "   Install from: https://wixtoolset.org/releases/" -ForegroundColor Yellow
        exit 1
    }
    
    # Create WiX source file
    $wixSource = @"
<?xml version="1.0" encoding="UTF-8"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
    <Product Id="*" Name="$AppName" Language="1033" Version="$AppVersion" 
             Manufacturer="$Publisher" UpgradeCode="12345678-1234-1234-1234-123456789012">
        
        <Package InstallerVersion="200" Compressed="yes" InstallScope="perMachine" />
        
        <MajorUpgrade DowngradeErrorMessage="A newer version of [ProductName] is already installed." />
        <MediaTemplate EmbedCab="yes" />

        <Feature Id="ProductFeature" Title="FileVault" Level="1">
            <ComponentGroupRef Id="ProductComponents" />
        </Feature>

        <Directory Id="TARGETDIR" Name="SourceDir">
            <Directory Id="ProgramFiles64Folder">
                <Directory Id="INSTALLFOLDER" Name="$AppName" />
            </Directory>
            <Directory Id="ProgramMenuFolder">
                <Directory Id="ApplicationProgramsFolder" Name="$AppName"/>
            </Directory>
        </Directory>

        <ComponentGroup Id="ProductComponents" Directory="INSTALLFOLDER">
            <Component Id="FileVaultExe" Guid="*">
                <File Id="FileVaultExe" Source="$ExePath" KeyPath="yes">
                    <Shortcut Id="ApplicationStartMenuShortcut" Directory="ApplicationProgramsFolder" 
                              Name="$AppName" WorkingDirectory="INSTALLFOLDER" Advertise="yes" />
                </File>
                <Environment Id="PATH" Name="PATH" Value="[INSTALLFOLDER]" 
                             Permanent="no" Part="last" Action="set" System="yes" />
            </Component>
        </ComponentGroup>
    </Product>
</Wix>
"@
    
    $wixFile = Join-Path $OutputDir "filevault.wxs"
    Set-Content -Path $wixFile -Value $wixSource -Encoding UTF8
    Write-Host "✓ Generated WiX source: $wixFile" -ForegroundColor Green
    
    # Compile WiX
    $wixObj = Join-Path $OutputDir "filevault.wixobj"
    & candle.exe -out $wixObj $wixFile
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ WiX compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    # Link MSI
    $msiPath = Join-Path $OutputDir "FileVault-$AppVersion-x64.msi"
    & light.exe -out $msiPath $wixObj
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ MSI linking failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "✓ MSI installer created: $msiPath" -ForegroundColor Green
    
} elseif ($Type -eq 'nsis') {
    # NSIS installer
    Write-Host "`nCreating NSIS installer..." -ForegroundColor Cyan
    
    # Check for NSIS
    $nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"
    if (-not (Test-Path $nsisPath)) {
        Write-Host "❌ Error: NSIS not found at: $nsisPath" -ForegroundColor Red
        Write-Host "   Install from: https://nsis.sourceforge.io/Download" -ForegroundColor Yellow
        Write-Host "   Or specify path with -NsisPath parameter" -ForegroundColor Yellow
        exit 1
    }
    
    # Create NSIS script
    $nsisScript = @"
; FileVault NSIS Installer Script
; Generated by packaging script

!define APP_NAME "$AppName"
!define APP_VERSION "$AppVersion"
!define PUBLISHER "$Publisher"
!define APP_URL "$AppUrl"
!define BUILD_DIR "$BuildDir"

!include "MUI2.nsh"

Name "`${APP_NAME}"
OutFile "$OutputDir\FileVault-`${APP_VERSION}-x64-setup.exe"
InstallDir "`$PROGRAMFILES64\`${APP_NAME}"
InstallDirRegKey HKLM "Software\`${APP_NAME}" "Install_Dir"
RequestExecutionLevel admin

; Modern UI Configuration
!define MUI_ABORTWARNING
!define MUI_ICON "`${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "`${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Pages
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"

; Version Information
VIProductVersion "`${APP_VERSION}.0"
VIAddVersionKey "ProductName" "`${APP_NAME}"
VIAddVersionKey "CompanyName" "`${PUBLISHER}"
VIAddVersionKey "FileDescription" "Professional File Encryption Tool"
VIAddVersionKey "FileVersion" "`${APP_VERSION}"
VIAddVersionKey "ProductVersion" "`${APP_VERSION}"

Section "Install"
    SetOutPath "`$INSTDIR"
    
    ; Copy executable
    File "`${BUILD_DIR}\filevault.exe"
    
    ; Copy dependencies (if any)
    File /nonfatal "`${BUILD_DIR}\*.dll"
    
    ; Write registry keys
    WriteRegStr HKLM "Software\`${APP_NAME}" "Install_Dir" "`$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "DisplayName" "`${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "UninstallString" "`$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "DisplayVersion" "`${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "Publisher" "`${PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "URLInfoAbout" "`${APP_URL}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}" "NoRepair" 1
    
    ; Add to PATH
    EnVar::SetHKLM
    EnVar::AddValue "PATH" "`$INSTDIR"
    
    ; Create shortcuts
    CreateDirectory "`$SMPROGRAMS\`${APP_NAME}"
    CreateShortcut "`$SMPROGRAMS\`${APP_NAME}\`${APP_NAME}.lnk" "`$INSTDIR\filevault.exe"
    CreateShortcut "`$SMPROGRAMS\`${APP_NAME}\Uninstall.lnk" "`$INSTDIR\uninstall.exe"
    
    ; Create uninstaller
    WriteUninstaller "`$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
    ; Remove files
    Delete "`$INSTDIR\filevault.exe"
    Delete "`$INSTDIR\*.dll"
    Delete "`$INSTDIR\uninstall.exe"
    RMDir "`$INSTDIR"
    
    ; Remove shortcuts
    Delete "`$SMPROGRAMS\`${APP_NAME}\*.lnk"
    RMDir "`$SMPROGRAMS\`${APP_NAME}"
    
    ; Remove from PATH
    EnVar::SetHKLM
    EnVar::DeleteValue "PATH" "`$INSTDIR"
    
    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\`${APP_NAME}"
    DeleteRegKey HKLM "Software\`${APP_NAME}"
SectionEnd
"@
    
    $nsisFile = Join-Path $OutputDir "filevault.nsi"
    Set-Content -Path $nsisFile -Value $nsisScript -Encoding UTF8
    Write-Host "✓ Generated NSIS script: $nsisFile" -ForegroundColor Green
    
    # Compile NSIS
    & $nsisPath $nsisFile
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ NSIS compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    $installerPath = Join-Path $OutputDir "FileVault-$AppVersion-x64-setup.exe"
    Write-Host "✓ NSIS installer created: $installerPath" -ForegroundColor Green
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "✅ Installer creation completed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
