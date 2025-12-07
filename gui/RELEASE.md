# FileVault GUI Release Workflow

This workflow produces signed, cross-platform GUI builds with Tauri.

## 0) Prerequisites
- Node.js 18+ and npm
- Rust stable (matching each target)
- Tauri prerequisites per OS: https://tauri.app/start/prerequisites/
- Install dependencies: `npm ci`

## 1) Version bump
1. Update app version in `src-tauri/tauri.conf.json` (`package.version`) and, if needed, `package.json`.
2. Update `CHANGELOG.md` (see repository root).

## 2) Build commands
Run from `gui/` unless noted.

### Windows (x64)
```pwsh
npm run tauri build
```
Output: `src-tauri/target/release/bundle/nsis/*.exe` (installer) and `bundle/msi` if configured.

### macOS (Intel/Apple Silicon)
```bash
# For universal app (requires both targets installed)
rustup target add aarch64-apple-darwin x86_64-apple-darwin
npm run tauri build -- --target universal-apple-darwin
```
Outputs under `src-tauri/target/universal-apple-darwin/release/bundle/macos/*.app` and `.dmg` if enabled.

### Linux (AppImage + deb/rpm if configured)
```bash
# Example for x86_64
rustup target add x86_64-unknown-linux-gnu
npm run tauri build -- --target x86_64-unknown-linux-gnu
```
Outputs under `src-tauri/target/x86_64-unknown-linux-gnu/release/bundle/` (AppImage, deb, rpm depending on toolchain).

### Additional targets
- Add the rust target via `rustup target add <triple>`
- Pass `--target <triple>` to `npm run tauri build`

## 3) Code signing
- Windows: configure `tauri.conf.json > bundle > windows > certificateThumbprint` or `digestAlgorithm`, and ensure cert is installed. Alternatively set `TAURI_PRIVATE_KEY`/`TAURI_KEY_PASSWORD` env vars.
- macOS: set `APPLE_ID`, `APPLE_PASSWORD`/`APPLE_APP_SPECIFIC_PASSWORD`, and signing identities per Tauri docs.
- Linux: optional; rpm/deb signing can be wired through `bundle > linux` config.

## 4) Artifacts & release packaging
- Collect installers/bundles from `src-tauri/target/<target>/release/bundle/`
- Verify launch:
  - `./filevault-gui.app/Contents/MacOS/filevault-gui --version`
  - `./FileVault_GUI-x86_64.AppImage --version`
  - Windows installer + installed exe launch
- Smoke test: theme switch, encrypt/decrypt happy-path, settings persistence.

## 5) Publish
1. Create a Git tag matching the version (e.g., `v1.1.1`).
2. Draft GitHub Release with checksums and platform artifacts.
3. Attach changelog excerpt and known issues.

## 6) Fast local smoke test (no signing)
```pwsh
npm run tauri dev   # quick UI check
npm run build       # ensures Tailwind/DaisyUI output is good
```

---
Keep this file alongside GUI sources to reuse across releases.
