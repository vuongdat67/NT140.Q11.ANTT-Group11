# FileVault GUI Testing Guide

## Recent Fixes (Dec 7, 2025)

### Issues Fixed:
1. **File Path Problem**: HTML `<input type="file">` không trả về full path
   - Fixed: Dùng Tauri dialog API (`@tauri-apps/plugin-dialog`)
   - Giờ FilePicker trả về absolute path chính xác

2. **Algorithm Name Case Sensitivity**: CLI expect lowercase
   - Fixed: Auto convert uppercase → lowercase (RSA-4096 → rsa-4096)
   - Áp dụng cho: encrypt, hash, compress, keygen

3. **Keygen Output Path Confusion**: User nghĩ là folder, thực ra là file prefix
   - Fixed: UI label "Output File Prefix", help text rõ ràng
   - Example: `C:\keys\mykey` → creates `mykey.pub` + `mykey.key`

4. **Enhanced Logging**: Thêm debug output trong Rust backend
   - Log executable path, argument count, từng argument
   - Show CLI stdout/stderr trong UI logs

## Testing Steps

### 1. Test Encrypt with File Paths Containing Spaces

```
1. Launch FileVault GUI
2. Go to Encrypt page
3. Select file: "FileVault-API 1.pdf" (có space trong tên)
4. Set output: "encrypted output.enc"
5. Algorithm: AES-256-GCM (sẽ auto lowercase)
6. Password: test1234
7. Click Encrypt
8. Check logs - should see:
   - Debug: Arg[0]: "encrypt"
   - Debug: Arg[1]: "C:\full\path\FileVault-API 1.pdf"
   - Success: "File encrypted successfully!"
```

### 2. Test Hash with Different Algorithms

```
1. Go to Hash page
2. Select file: "test file.txt"
3. Try each algorithm:
   - SHA256 → automatically converted to "sha256"
   - BLAKE2B-512 → converted to "blake2b-512"
4. Verify hash result displays
5. Test Copy to Clipboard button
```

### 3. Test Keygen with Correct Output Path

```
1. Go to Keygen page
2. Algorithm: RSA-4096
3. Output File Prefix: C:\keys\mykey
   - NOTE: Enter prefix WITHOUT extension
   - Will create: mykey.pub and mykey.key
4. Password: securepass123
5. Click Generate
6. Check output directory for .pub and .key files
```

### 4. Test Other Features

#### Decrypt
- Select .enc file from encrypt test
- Enter same password
- Verify decrypted file matches original

#### Sign/Verify
- Use keypair from keygen test
- Sign a document
- Verify signature with public key

#### Steganography
- Hide: container + payload → output
- Extract: output → recovered payload

#### Archive
- Create: multiple files → .fva archive
- Extract: .fva → folder with files

#### Compress
- Test: ZLIB, LZMA, BZIP2
- Verify compression ratio displayed
- Test decompress

## Known Issues

### If File Path Still Fails:
1. Check Tauri developer console (F12)
2. Look for backend logs showing arguments
3. Verify path format (Windows backslash vs forward slash)

### If Algorithm Not Found:
- Ensure algorithm names are lowercase in CLI
- Check `encrypt_cmd.cpp` for valid algorithm list
- GUI should auto-convert, but verify in logs

### If Keygen Output Error:
- Don't use folder picker (removed)
- Type full file prefix: `C:\keys\mykey`
- Don't add extensions (.pub/.key)

## Debug Tools

### View Rust Backend Logs:
```powershell
# Run GUI from terminal to see println! output
cd D:\code\filevault\gui\src-tauri
cargo tauri dev
```

### View Browser Console:
- Press F12 in GUI window
- Check Console tab for JavaScript errors
- Check Network tab for Tauri invoke calls

### View CLI Directly:
```powershell
# Test CLI command manually
$exe = "D:\code\filevault\build\build\Release\bin\release\filevault.exe"
& $exe encrypt "test file.txt" "output.enc" -a aes-256-gcm -p test123
```

## Success Criteria

✅ File picker returns full absolute path
✅ Paths with spaces work correctly
✅ Algorithm names auto-convert to lowercase
✅ Keygen creates .pub and .key files
✅ All operations log detailed error messages
✅ CLI stdout/stderr visible in GUI logs

## Report Issues

If you find bugs:
1. Note exact steps to reproduce
2. Copy full error message from logs
3. Check developer console (F12) for JS errors
4. If possible, test same command in CLI directly
5. Include file paths and settings used
