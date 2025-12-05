# FileVault GUI Development Plan

**Date:** December 5, 2025  
**Version:** 1.1.0  
**Goal:** Tạo GUI desktop app dễ dùng, đẹp, nhanh, cross-platform

---

## Stack Comparison

### 1. **Tauri** ⭐ RECOMMENDED
**Pros:**
- ✅ Rust backend (an toàn, nhanh)
- ✅ Web frontend (HTML/CSS/JS/React/Vue)
- ✅ Bundle size NHỎ (~5MB vs Electron ~120MB)
- ✅ RAM usage THẤP (Rust + native webview)
- ✅ Cross-platform (Windows/Mac/Linux)
- ✅ Có thể reuse CLI code (gọi executable)
- ✅ Modern UI với Tailwind CSS
- ✅ Security tốt (Rust + permission system)

**Cons:**
- ⚠️ Cần học Rust cơ bản
- ⚠️ Ecosystem nhỏ hơn Electron

**Tech Stack:**
- Backend: Rust (Tauri)
- Frontend: React/Vite + TypeScript
- UI: shadcn/ui + Tailwind CSS
- Icons: Lucide React

### 2. **Electron**
**Pros:**
- ✅ Mature ecosystem
- ✅ VS Code dùng Electron (proven)
- ✅ Easy to start (chỉ cần JS)
- ✅ Nhiều libraries

**Cons:**
- ❌ Bundle size LỚN (~120MB+)
- ❌ RAM usage CAO
- ❌ Slow startup

### 3. **Qt (C++)**
**Pros:**
- ✅ Native performance
- ✅ Professional look

**Cons:**
- ❌ KHÓ học
- ❌ QML phức tạp
- ❌ License issues (GPL/Commercial)
- ❌ Styling khó

### 4. **Neutralinojs**
**Pros:**
- ✅ Nhẹ (~2MB)
- ✅ Native webview

**Cons:**
- ❌ Ecosystem nhỏ
- ❌ Features ít

---

## ⭐ FINAL DECISION: Tauri

**Lý do chọn Tauri:**
1. **Performance** - Bundle nhỏ, RAM thấp
2. **Modern** - Stack hiện đại (React/TypeScript)
3. **Security** - Rust backend an toàn
4. **Reusability** - Gọi CLI executable có sẵn
5. **UI** - Dễ làm đẹp với Tailwind + shadcn

---

## Implementation Plan

### Phase 1: Setup (1-2 days)
```bash
# Install Tauri
npm create tauri-app@latest

# Choose:
- Project name: filevault-gui
- Package manager: npm
- UI template: React + TypeScript
- UI flavor: Vite
```

**Tech Stack:**
- Tauri 2.x
- React 18
- TypeScript
- Vite
- Tailwind CSS
- shadcn/ui components
- Zustand (state management)
- React Query (async state)

### Phase 2: Core Features (3-5 days)

**UI Components:**
```
src/
├── components/
│   ├── Sidebar.tsx          # Navigation
│   ├── FileDropZone.tsx     # Drag & drop
│   ├── AlgorithmSelector.tsx
│   ├── ProgressBar.tsx
│   └── ResultDisplay.tsx
├── pages/
│   ├── Encrypt.tsx
│   ├── Decrypt.tsx
│   ├── Hash.tsx
│   ├── Keygen.tsx
│   ├── Sign.tsx
│   ├── Verify.tsx
│   └── Settings.tsx
├── lib/
│   ├── cli.ts              # CLI wrapper
│   └── utils.ts
└── App.tsx
```

**Backend Integration:**
```rust
// src-tauri/src/main.rs
#[tauri::command]
async fn encrypt_file(
    input: String,
    output: String,
    algorithm: String,
    password: String
) -> Result<String, String> {
    // Call filevault.exe CLI
    let output = Command::new("filevault")
        .args(&["encrypt", &input, output, "-a", &algorithm, "-p", &password])
        .output()
        .await?;
    
    Ok(String::from_utf8(output.stdout)?)
}
```

### Phase 3: UI Design (2-3 days)

**Color Scheme:**
- Dark mode primary
- Accent: Blue (#3B82F6)
- Success: Green (#10B981)
- Error: Red (#EF4444)

**Layout:**
```
┌─────────────────────────────────────────┐
│  FileVault                    ⚙️ 🌙 ❓  │
├──────┬──────────────────────────────────┤
│      │  Drop files here                 │
│ 🔒   │  or click to select              │
│ Enc  │                                  │
│      │  [Selected: document.txt]        │
├──────┤                                  │
│ 🔓   │  Algorithm: AES-256-GCM  ▼       │
│ Dec  │  Mode:      Standard     ▼       │
├──────┤  Password:  ••••••••••           │
│ #️⃣   │                                  │
│ Hash │  [Encrypt File]                  │
├──────┤                                  │
│ 🔑   │  Progress: ████████░░ 80%        │
│ Keys │                                  │
├──────┤                                  │
│ ✍️    │                                  │
│ Sign │                                  │
└──────┴──────────────────────────────────┘
```

### Phase 4: Advanced Features (2-3 days)
- [ ] Batch processing
- [ ] History/recent files
- [ ] Presets management
- [ ] Drag & drop multiple files
- [ ] Context menu integration
- [ ] Auto-update

### Phase 5: Testing & Polish (2 days)
- [ ] Unit tests
- [ ] E2E tests
- [ ] Performance optimization
- [ ] Icon design
- [ ] Installer creation

---

## Development Timeline

**Total: ~10-15 days**

| Phase | Duration | Tasks |
|-------|----------|-------|
| Setup | 1-2 days | Tauri + React + UI components |
| Core | 3-5 days | All crypto operations |
| UI | 2-3 days | Design + styling |
| Advanced | 2-3 days | Batch, history, presets |
| Polish | 2 days | Testing + packaging |

---

## Getting Started

### 1. Install Prerequisites
```bash
# Rust
winget install Rustlang.Rust.MSVC

# Node.js (already installed)
node --version  # v20+

# Tauri CLI
cargo install tauri-cli
```

### 2. Create Project
```bash
npm create tauri-app@latest
cd filevault-gui
npm install
```

### 3. Install UI Dependencies
```bash
npm install @radix-ui/react-icons
npm install @radix-ui/react-dropdown-menu
npm install @radix-ui/react-dialog
npm install tailwindcss-animate
npm install zustand
npm install @tanstack/react-query
npm install clsx tailwind-merge
```

### 4. Add shadcn/ui
```bash
npx shadcn-ui@latest init
npx shadcn-ui@latest add button
npx shadcn-ui@latest add input
npx shadcn-ui@latest add select
npx shadcn-ui@latest add progress
npx shadcn-ui@latest add card
npx shadcn-ui@latest add tabs
```

### 5. Development
```bash
npm run tauri dev    # Run dev mode
npm run tauri build  # Build release
```

---

## Folder Structure
```
filevault-gui/
├── src/                    # React frontend
│   ├── components/
│   ├── pages/
│   ├── lib/
│   ├── App.tsx
│   └── main.tsx
├── src-tauri/             # Rust backend
│   ├── src/
│   │   └── main.rs
│   ├── icons/
│   └── Cargo.toml
├── public/
├── package.json
└── README.md
```

---

## CLI Integration Strategy

**Option 1: Call External Executable** (RECOMMENDED)
```typescript
// lib/cli.ts
import { Command } from '@tauri-apps/api/shell';

export async function encryptFile(
  input: string,
  output: string,
  options: EncryptOptions
) {
  const args = [
    'encrypt',
    input,
    output,
    '-a', options.algorithm,
    '-p', options.password,
  ];
  
  const command = new Command('filevault', args);
  const output = await command.execute();
  
  return output;
}
```

**Option 2: Rust FFI** (Future)
- Link C++ CLI library directly
- Faster, no subprocess overhead
- More complex setup

---

## UI Mockups

### Encrypt Page
```
┌─────────────────────────────────────────┐
│  Encrypt File                           │
├─────────────────────────────────────────┤
│                                         │
│  📄 Drop file here or click to browse  │
│                                         │
│  Selected: document.pdf (2.5 MB)       │
│                                         │
│  Algorithm:  [AES-256-GCM       ▼]     │
│  Mode:       [Standard          ▼]     │
│             ○ Basic                    │
│             ● Standard                 │
│             ○ Advanced                 │
│                                         │
│  Password:   [••••••••••]              │
│  Confirm:    [••••••••••]              │
│                                         │
│  ☑ Compress with LZMA                  │
│  ☐ Delete original after encryption    │
│                                         │
│              [Encrypt File]             │
│                                         │
└─────────────────────────────────────────┘
```

### Hash Page
```
┌─────────────────────────────────────────┐
│  Hash File                              │
├─────────────────────────────────────────┤
│                                         │
│  File: document.txt                     │
│                                         │
│  Algorithm: [SHA256 ▼]                 │
│  Format:    [Hex ▼]                    │
│                                         │
│  Result:                                │
│  ┌────────────────────────────────────┐│
│  │ a3f5b... (click to copy)          ││
│  └────────────────────────────────────┘│
│                                         │
│  [Calculate Hash] [Copy] [Save]        │
│                                         │
└─────────────────────────────────────────┘
```

---

## Next Steps

1. ✅ Review and approve this plan
2. ⏳ Install Tauri prerequisites
3. ⏳ Create project scaffolding
4. ⏳ Implement Encrypt/Decrypt pages
5. ⏳ Add remaining features
6. ⏳ Testing and refinement

---

## Notes

- **Priority**: Ưu tiên encrypt/decrypt/hash trước
- **UX**: Đơn giản, ít click, drag & drop
- **Design**: Dark mode, modern, clean
- **Performance**: Mượt, < 100ms response
- **Bundle**: Target < 10MB installer

**Question for User:**
- Có muốn tích hợp với File Explorer context menu không? (Right-click → FileVault → Encrypt)
- Có cần tính năng scheduled encryption không? (Auto-encrypt folders)
- Có muốn cloud sync settings không?
