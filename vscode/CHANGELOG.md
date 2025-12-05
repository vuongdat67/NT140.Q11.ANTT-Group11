# Change Log

All notable changes to the "FileVault" extension will be documented in this file.

## [1.0.5] - 2024-12-05

### Fixed
- **Stego Extract UX**: Extension now properly detects original filename before showing save dialog
  - Extracts to temp first, reads metadata, then shows save dialog with correct filename
  - "Open File" button now works correctly (opens the actual extracted file)
  - No more file type restrictions - save dialog shows "All Files" filter
  - Default filename is the original embedded filename (e.g., Lab5.pdf)

### Technical Details
- Extract workflow: temp extraction → read metadata → show save dialog → move to final location
- Removed confusing file type filters that limited user's choice
- Save dialog now defaults to same directory as stego image

## [1.0.4] - 2024-12-05

### Fixed
- **CRITICAL**: Decompress now works correctly - fixed `shell: true` causing path escaping issues
- **CRITICAL**: Stego extract now automatically preserves original filename and extension
- Extension commands now work with paths containing spaces and special characters
- Subprocess spawning improved - proper argument array passing without shell escaping

### Added
- **Stego Filename Metadata**: Files embedded via steganography now include metadata
  - Format: `[2 bytes: filename_length][filename][original_data]`
  - Extract operation automatically restores original filename (e.g., Lab5.pdf → Lab5.pdf)
  - Backward compatible: extracts old stego files without metadata

### Technical Details
- Changed `child_process.spawn()` from `shell: true` to `shell: false`
- Explicit stdio configuration for better subprocess control
- Type-safe TypeScript with proper Buffer and event handler types

## [1.0.3] - 2024-12-05

### Fixed
- **CRITICAL**: Archive creation no longer hangs - password now required upfront
- **Stego Extract**: File extension now customizable (was hardcoded to .dat)
- **Decompress**: Fixed CLI command syntax (works with all 3 algorithms now)
- Archive file filter changed from .fvlt to .fva (correct extension)
- Password validation: minimum 6 characters required for archive

### Changed
- Stego extract now shows file type filters (PDF, images, videos, etc.)
- Password prompt moved before file save dialog to prevent CLI blocking

## [1.0.2] - 2024-12-04

### Added
- **BZIP2/BZIP3 Support**: Implemented full BZIP3 compression support using libbz3 API
- BZIP2 now available in compress, decompress, and archive commands
- Better compression ratios than zlib with BZIP3 (typically 1.5-2.5%)

### Changed
- Updated BZIP2 implementation from disabled stub to full BZIP3 API
- Extension now shows BZIP2 with warning note "(⚠️ Temporarily disabled)" for transparency
- Improved buffer size estimation for BZIP3 decompression (100x-1000x dynamic sizing)

### Fixed
- BZIP3 decompress buffer sizing issue (BZ3_ERR_DATA_TOO_BIG)
- Compression field naming (processing_time_ms instead of compression_time)

## [1.0.1] - 2024-12-04

### Changed
- Attempted to remove BZIP2 (reverted in 1.0.2 after implementing BZIP3)

## [1.0.0] - 2024-12-01

### Added
- Initial release
- File encryption and decryption with multiple algorithms
- Support for Post-Quantum Cryptography (Kyber)
- Archive creation and extraction
- Steganography (LSB embedding in images)
- File compression (LZMA, ZLIB)
- File hashing with multiple algorithms
- Benchmark tools
- Key generation utilities
