# FileVault GUI - Setup Guide

Hướng dẫn chi tiết để setup và chạy FileVault GUI.

## Bước 1: Copy FileVault CLI

Trước tiên, cần copy `filevault.exe` vào thư mục bundle:

```powershell
# Từ thư mục gui
cd D:\code\filevault\gui

# Copy executable
Copy-Item "..\build\build\Release\bin\release\filevault.exe" "src-tauri\bin\" -Force
```

Kiểm tra file đã được copy:
```powershell
Test-Path "src-tauri\bin\filevault.exe"
# Phải trả về True
```

## Bước 2: Install Dependencies

### Frontend dependencies
```powershell
npm install
```

### Rust dependencies (tự động khi build)
Tauri sẽ tự động cài đặt Rust dependencies khi bạn chạy `npm run tauri dev` lần đầu.

## Bước 3: Chạy Development Mode

```powershell
npm run tauri dev
```

Lệnh này sẽ:
1. Build Rust backend (`src-tauri/`)
2. Start Vite dev server (React frontend)
3. Mở cửa sổ ứng dụng

**Lưu ý**: Lần đầu tiên có thể mất 2-5 phút để compile Rust code.

## Bước 4: Test Chức Năng

### Test Encrypt
1. Click vào "Encrypt" trong sidebar
2. Chọn một file bất kỳ
3. Nhập password (tối thiểu 8 ký tự)
4. Click "Encrypt File"
5. Kiểm tra logs phía dưới

### Test Hash
1. Click vào "Hash"
2. Chọn file
3. Click "Compute Hash"
4. Copy hash result

## Bước 5: Build Production

```powershell
npm run tauri build
```

Installer sẽ được tạo ở:
- MSI: `src-tauri\target\release\bundle\msi\FileVault_1.0.0_x64_en-US.msi`
- NSIS: `src-tauri\target\release\bundle\nsis\FileVault_1.0.0_x64-setup.exe`

## Troubleshooting

### Lỗi: "FileVault executable not found"

**Nguyên nhân**: File `filevault.exe` không tồn tại ở đúng vị trí.

**Giải pháp**:
```powershell
# Kiểm tra file CLI có tồn tại không
Test-Path "..\build\build\Release\bin\release\filevault.exe"

# Copy lại
Copy-Item "..\build\build\Release\bin\release\filevault.exe" "src-tauri\bin\" -Force
```

### Lỗi: "failed to bundle project"

**Nguyên nhân**: Thiếu Rust toolchain hoặc WebView2.

**Giải pháp**:
```powershell
# Install Rust nếu chưa có
winget install Rustlang.Rust.MSVC

# Update Rust
rustup update

# Kiểm tra version
rustc --version
```

### Lỗi: TypeScript compilation errors

**Giải pháp**:
```powershell
# Clean và reinstall
Remove-Item -Recurse -Force node_modules
npm install
```

### Lỗi: "Cannot find module '@tauri-apps/api'"

**Giải pháp**:
```powershell
npm install @tauri-apps/api
```

### Ứng dụng không gọi được CLI

**Kiểm tra**:
1. Mở Developer Tools trong app (F12 hoặc Ctrl+Shift+I)
2. Xem Console có lỗi gì không
3. Kiểm tra Rust logs:
   ```powershell
   # Trong terminal khi chạy dev mode
   # Sẽ thấy logs như:
   # Executing: "D:\code\filevault\gui\src-tauri\bin\filevault.exe" with args: ["encrypt", ...]
   ```

## Development Tips

### Hot Reload
- **Frontend**: Tự động reload khi sửa `.tsx`, `.ts`, `.css`
- **Backend**: Cần restart (`Ctrl+C` rồi `npm run tauri dev` lại) khi sửa `.rs`

### Debug Rust
```rust
// Thêm vào lib.rs để debug
println!("Debug: {:?}", variable);
```

Xem output trong terminal nơi chạy `npm run tauri dev`.

### Debug TypeScript/React
- Mở DevTools: `F12` hoặc `Ctrl+Shift+I`
- Console tab: Xem logs
- Network tab: Không có (vì không có API calls)
- Sources tab: Debug với breakpoints

### Xem Structure
```powershell
# List all files trong src
Get-ChildItem -Recurse src | Select-Object FullName
```

## Architecture Overview

```
User clicks "Encrypt"
    ↓
React component (Encrypt.tsx)
    ↓
TypeScript helper (lib/cli.ts → encryptFile())
    ↓
Tauri invoke (invoke('run_filevault_command', { args }))
    ↓
Rust backend (src-tauri/src/lib.rs → run_filevault_command())
    ↓
Execute CLI (Command::new("filevault.exe").args(...).output())
    ↓
Return result
    ↓
Display in UI (LogPanel, ProgressBar)
```

## Next Steps

Sau khi setup xong:
1. ✅ Test tất cả các trang (Encrypt, Decrypt, Hash, etc.)
2. ✅ Build production installer
3. ✅ Test installer trên máy clean
4. ✅ Deploy hoặc share với người dùng

## File Paths Reference

**Development**:
- Frontend source: `D:\code\filevault\gui\src\`
- Backend source: `D:\code\filevault\gui\src-tauri\src\`
- CLI executable: `D:\code\filevault\gui\src-tauri\bin\filevault.exe`

**Production (sau khi install)**:
- App location: `C:\Program Files\FileVault\`
- CLI bundled: `C:\Program Files\FileVault\bin\filevault.exe`
- User data: `%APPDATA%\com.filevault.app\`

## Useful Commands

```powershell
# Development
npm run tauri dev

# Build production
npm run tauri build

# Type check only
npm run check

# Lint
npm run lint

# Clean build artifacts
Remove-Item -Recurse -Force src-tauri\target

# Update dependencies
npm update
cargo update
```

## Contact & Support

Nếu gặp vấn đề:
1. Kiểm tra logs trong terminal
2. Kiểm tra DevTools Console (F12)
3. Đọc lại hướng dẫn trong README.md
4. Tạo issue trên GitHub repository
