# FileVault - UTF-8 Encoding Normalization Script
# Automatically converts files to UTF-8 and creates backups

param(
    [switch]$DryRun = $false,
    [switch]$SkipBackup = $false,
    [string]$BackupDir = ".encoding_backup"
)

$ErrorActionPreference = "Continue"

function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

function Test-TextFile {
    param([string]$Path)
    
    $extension = [System.IO.Path]::GetExtension($Path).ToLower()
    $textExtensions = @(
        '.cpp', '.hpp', '.h', '.c', '.cc', '.cxx',
        '.md', '.txt', '.rst', '.adoc',
        '.json', '.yaml', '.yml', '.toml', '.ini', '.cfg', '.conf',
        '.cmake', '.sh', '.bash', '.ps1', '.bat', '.cmd',
        '.xml', '.html', '.htm', '.css', '.js', '.ts'
    )
    
    return $textExtensions -contains $extension
}

function Get-FileEncoding {
    param([string]$Path)
    
    try {
        $bytes = [System.IO.File]::ReadAllBytes($Path)
        
        if ($bytes.Length -eq 0) {
            return "Empty"
        }
        
        # Check BOM
        if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
            return "UTF8-BOM"
        }
        if ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFF -and $bytes[1] -eq 0xFE) {
            return "UTF16-LE"
        }
        if ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFE -and $bytes[1] -eq 0xFF) {
            return "UTF16-BE"
        }
        
        # Check for null bytes (UTF-16 or binary)
        if ($bytes -contains 0x00) {
            return "Binary or UTF16"
        }
        
        # Try to detect UTF-8 without BOM
        $isValidUtf8 = $true
        for ($i = 0; $i -lt $bytes.Length; $i++) {
            if ($bytes[$i] -gt 0x7F) {
                # Multi-byte UTF-8 sequence
                if ($bytes[$i] -band 0xE0 -eq 0xC0) {
                    # 2-byte sequence
                    if ($i + 1 -ge $bytes.Length -or ($bytes[$i + 1] -band 0xC0) -ne 0x80) {
                        $isValidUtf8 = $false
                        break
                    }
                    $i += 1
                } elseif ($bytes[$i] -band 0xF0 -eq 0xE0) {
                    # 3-byte sequence
                    if ($i + 2 -ge $bytes.Length -or 
                        ($bytes[$i + 1] -band 0xC0) -ne 0x80 -or 
                        ($bytes[$i + 2] -band 0xC0) -ne 0x80) {
                        $isValidUtf8 = $false
                        break
                    }
                    $i += 2
                } elseif ($bytes[$i] -band 0xF8 -eq 0xF0) {
                    # 4-byte sequence
                    if ($i + 3 -ge $bytes.Length -or 
                        ($bytes[$i + 1] -band 0xC0) -ne 0x80 -or 
                        ($bytes[$i + 2] -band 0xC0) -ne 0x80 -or 
                        ($bytes[$i + 3] -band 0xC0) -ne 0x80) {
                        $isValidUtf8 = $false
                        break
                    }
                    $i += 3
                } else {
                    $isValidUtf8 = $false
                    break
                }
            }
        }
        
        if ($isValidUtf8) {
            return "UTF8"
        } else {
            return "Unknown/ANSI"
        }
    } catch {
        return "Error: $_"
    }
}

function Convert-ToUTF8 {
    param(
        [string]$Path,
        [string]$CurrentEncoding
    )
    
    try {
        # Read file with detected encoding
        $content = switch ($CurrentEncoding) {
            "UTF8-BOM" { 
                [System.IO.File]::ReadAllText($Path, [System.Text.UTF8Encoding]::new($true))
            }
            "UTF16-LE" { 
                [System.IO.File]::ReadAllText($Path, [System.Text.UnicodeEncoding]::new($false, $true))
            }
            "UTF16-BE" { 
                [System.IO.File]::ReadAllText($Path, [System.Text.UnicodeEncoding]::new($true, $true))
            }
            "Unknown/ANSI" {
                [System.IO.File]::ReadAllText($Path, [System.Text.Encoding]::Default)
            }
            default {
                [System.IO.File]::ReadAllText($Path)
            }
        }
        
        # Write as UTF-8 without BOM
        $utf8NoBom = [System.Text.UTF8Encoding]::new($false)
        [System.IO.File]::WriteAllText($Path, $content, $utf8NoBom)
        
        return $true
    } catch {
        Write-ColorOutput "Error converting ${Path}: $_" "Red"
        return $false
    }
}

# Main script
Write-ColorOutput "╔════════════════════════════════════════════════╗" "Cyan"
Write-ColorOutput "║   FileVault - UTF-8 Encoding Normalization    ║" "Cyan"
Write-ColorOutput "╚════════════════════════════════════════════════╝" "Cyan"
Write-Host ""

if ($DryRun) {
    Write-ColorOutput "[DRY RUN MODE] No files will be modified" "Yellow"
    Write-Host ""
}

# Create backup directory
if (-not $SkipBackup -and -not $DryRun) {
    if (-not (Test-Path $BackupDir)) {
        New-Item -ItemType Directory -Path $BackupDir | Out-Null
        Write-ColorOutput "Created backup directory: $BackupDir" "Green"
    }
}

# Exclude directories
$excludeDirs = @(
    'build', 'build-*', '.git', '.vs', '.vscode', 
    'bin', 'obj', 'node_modules', '__pycache__',
    $BackupDir, 'bintest'
)

# Get all text files
Write-ColorOutput "Scanning repository for text files..." "Cyan"
$files = Get-ChildItem -Path . -Recurse -File | Where-Object {
    $exclude = $false
    foreach ($dir in $excludeDirs) {
        if ($_.FullName -like "*\$dir\*" -or $_.FullName -like "*/$dir/*") {
            $exclude = $true
            break
        }
    }
    -not $exclude -and (Test-TextFile $_.FullName)
}

Write-ColorOutput "Found $($files.Count) text files" "Green"
Write-Host ""

# Analyze files
$stats = @{
    Total = 0
    UTF8 = 0
    UTF8BOM = 0
    UTF16 = 0
    ANSI = 0
    Binary = 0
    Errors = 0
    Converted = 0
}

$filesToConvert = @()

Write-ColorOutput "Analyzing file encodings..." "Cyan"
Write-Host ""

foreach ($file in $files) {
    $stats.Total++
    $relativePath = $file.FullName.Replace((Get-Location).Path + '\', '')
    $encoding = Get-FileEncoding -Path $file.FullName
    
    switch -Wildcard ($encoding) {
        "UTF8" {
            $stats.UTF8++
            Write-ColorOutput "  ✓ $relativePath (UTF-8)" "Green"
        }
        "UTF8-BOM" {
            $stats.UTF8BOM++
            Write-ColorOutput "  ⚠ $relativePath (UTF-8 with BOM - will remove)" "Yellow"
            $filesToConvert += @{Path = $file.FullName; RelPath = $relativePath; Encoding = $encoding}
        }
        "UTF16*" {
            $stats.UTF16++
            Write-ColorOutput "  ⚠ $relativePath ($encoding - will convert)" "Yellow"
            $filesToConvert += @{Path = $file.FullName; RelPath = $relativePath; Encoding = $encoding}
        }
        "Unknown/ANSI" {
            $stats.ANSI++
            Write-ColorOutput "  ⚠ $relativePath (ANSI/Unknown - will convert)" "Yellow"
            $filesToConvert += @{Path = $file.FullName; RelPath = $relativePath; Encoding = $encoding}
        }
        "Binary*" {
            $stats.Binary++
            Write-ColorOutput "  ⊗ $relativePath (Binary - skipping)" "DarkGray"
        }
        default {
            $stats.Errors++
            Write-ColorOutput "  ✗ $relativePath (Error: $encoding)" "Red"
        }
    }
}

Write-Host ""
Write-ColorOutput "═══════════════════ Summary ═══════════════════" "Cyan"
Write-ColorOutput "Total files scanned:     $($stats.Total)" "White"
Write-ColorOutput "Already UTF-8 (no BOM):  $($stats.UTF8)" "Green"
Write-ColorOutput "UTF-8 with BOM:          $($stats.UTF8BOM)" "Yellow"
Write-ColorOutput "UTF-16:                  $($stats.UTF16)" "Yellow"
Write-ColorOutput "ANSI/Unknown:            $($stats.ANSI)" "Yellow"
Write-ColorOutput "Binary/Skipped:          $($stats.Binary)" "DarkGray"
Write-ColorOutput "Files to convert:        $($filesToConvert.Count)" "Cyan"
Write-Host ""

if ($filesToConvert.Count -eq 0) {
    Write-ColorOutput "✓ All text files are already in UTF-8 without BOM!" "Green"
    exit 0
}

if ($DryRun) {
    Write-ColorOutput "[DRY RUN] Would convert $($filesToConvert.Count) files" "Yellow"
    Write-ColorOutput "Run without -DryRun to apply changes" "Yellow"
    exit 0
}

# Confirm conversion
Write-ColorOutput "Ready to convert $($filesToConvert.Count) files to UTF-8 (no BOM)" "Cyan"
$confirm = Read-Host "Proceed? [Y/n]"
if ($confirm -ne "" -and $confirm -ne "Y" -and $confirm -ne "y") {
    Write-ColorOutput "Conversion cancelled" "Yellow"
    exit 0
}

Write-Host ""
Write-ColorOutput "Converting files..." "Cyan"

foreach ($fileInfo in $filesToConvert) {
    Write-Host "  Converting: $($fileInfo.RelPath)..." -NoNewline
    
    # Create backup
    if (-not $SkipBackup) {
        $backupPath = Join-Path $BackupDir ([System.IO.Path]::GetFileName($fileInfo.Path))
        $counter = 1
        while (Test-Path $backupPath) {
            $backupPath = Join-Path $BackupDir "$([System.IO.Path]::GetFileNameWithoutExtension($fileInfo.Path))_$counter$([System.IO.Path]::GetExtension($fileInfo.Path))"
            $counter++
        }
        Copy-Item -Path $fileInfo.Path -Destination $backupPath -Force
    }
    
    # Convert
    if (Convert-ToUTF8 -Path $fileInfo.Path -CurrentEncoding $fileInfo.Encoding) {
        $stats.Converted++
        Write-ColorOutput " ✓" "Green"
    } else {
        Write-ColorOutput " ✗" "Red"
    }
}

Write-Host ""
Write-ColorOutput "═══════════════════ Complete ══════════════════" "Cyan"
Write-ColorOutput "Successfully converted: $($stats.Converted) files" "Green"

if (-not $SkipBackup) {
    Write-ColorOutput "Backups saved to: $BackupDir" "Cyan"
    Write-Host ""
    Write-ColorOutput "To restore backups if needed:" "Yellow"
    Write-ColorOutput "  Copy-Item $BackupDir\* -Destination . -Force" "Yellow"
}

Write-Host ""
Write-ColorOutput "✓ Encoding normalization complete!" "Green"
Write-Host ""
Write-ColorOutput "Next steps:" "Cyan"
Write-ColorOutput "  1. Review changes: git diff" "White"
Write-ColorOutput "  2. Test build: cmake --build build" "White"
Write-ColorOutput "  3. Commit: git add . && git commit -m 'chore: normalize file encodings to UTF-8'" "White"
