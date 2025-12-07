# 🚀 FileVault GUI - Quick Start

## ⚡ Chạy Ngay (3 Lệnh)

```powershell
cd D:\code\filevault\gui
npm install
npm run tauri dev
```

Xong! Ứng dụng sẽ mở trong vài giây (lần đầu có thể mất 2-3 phút để compile Rust).

## 📸 Screenshots

### Dashboard
- Tổng quan số liệu
- Quick start guide
- Recent activity

### Encrypt Page
1. Chọn file
2. Chọn algorithm (AES-256-GCM recommended)
3. Nhập password (min 8 chars)
4. Click "Encrypt File"
5. Xem logs real-time

### Hash Page
1. Chọn file
2. Chọn algorithm (SHA256, BLAKE3, etc.)
3. Click "Compute Hash"
4. Copy hash result

## 🎯 Common Tasks

### Encrypt một file
```
1. Click "Encrypt" trong sidebar
2. Browse file (hoặc drag & drop - coming soon)
3. Nhập password
4. Optional: Enable compression
5. Click "Encrypt File"
```

### Verify signature
```
1. Click "Verify"
2. Chọn file gốc
3. Chọn file signature (.sig)
4. Chọn public key (.pem)
5. Click "Verify Signature"
→ Sẽ hiện ✅ Valid hoặc ❌ Invalid
```

### Create archive
```
1. Click "Archive"
2. Chọn "Create Archive"
3. Add files (click + nhiều lần)
4. Chọn compression (LZMA recommended)
5. Optional: Enable encryption + password
6. Click "Create Archive"
```

## 🔧 Build Installer

```powershell
npm run tauri build
```

Installer sẽ ở:
- `src-tauri\target\release\bundle\msi\FileVault_1.0.0_x64_en-US.msi`
- `src-tauri\target\release\bundle\nsis\FileVault_1.0.0_x64-setup.exe`

## 📚 More Info

- Chi tiết setup: `SETUP.md`
- Full documentation: `README.md`
- Build summary: `BUILD_SUMMARY.md`

## 🎉 Enjoy!

FileVault GUI is ready to use. Have fun encrypting! 🔒
