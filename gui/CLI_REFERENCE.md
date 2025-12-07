# FileVault GUI - CLI Command Reference

## Fixed Based on Actual CLI Help

### ✅ Encrypt
```
filevault encrypt <input> <output> -a <algorithm> -p <password> -m <mode> --kdf <kdf> --compression <algo>
```
- Algorithm: aes-256-gcm, aes-256-cbc, chacha20-poly1305, camellia-256-gcm (lowercase)
- Mode: basic, standard, advanced
- KDF: argon2id, argon2i, pbkdf2-sha256, pbkdf2-sha512, scrypt (lowercase)
- Compression: zlib, bzip2, lzma, none (lowercase)

### ✅ Decrypt
```
filevault decrypt <input> <output> -p <password>
```
- No `--verify` flag exists
- Just input, output, password

### ✅ Hash
```
filevault hash <input> -a <algorithm> --format <format>
```
- Algorithm: sha256, sha512, sha3-256, sha3-512, blake2b, blake3 (lowercase)
- Format: hex, base64, binary (lowercase)
- Parameter is `--format` NOT `-f`

### ✅ Keygen
```
filevault keygen -a <algorithm> -o <output_prefix>
```
- Algorithm includes key size: rsa-2048, rsa-3072, rsa-4096, ecc-p256, ecc-p384, ecc-p521 (lowercase)
- **NO --key-size parameter** - size is in algorithm name
- **NO -p password** - keys generated unencrypted
- Output: creates `prefix.pub` and `prefix.key`

### ✅ Sign
```
filevault sign <file> <private-key> -o <output>
```
- Positional args: file then private-key
- `-o` for output signature file
- Algorithm optional: rsa, ecc, ed25519

### ✅ Verify
```
filevault verify <file> <signature> <public-key>
```
- Three positional args, no flags
- No `-k` for public key - it's positional

### ✅ Stego
```
filevault stego embed <input> <cover> <output>
filevault stego extract <stego> <output>
```
- **Subcommands**: `embed` or `extract`
- NOT `stego-hide` / `stego-extract`
- Embed: input (secret) cover (image) output
- Extract: stego (image) output

### ✅ Archive
```
filevault archive create <files...> -o <output> -c <compression> -p <password>
filevault archive extract <archive> -o <output> -p <password>
```
- **Subcommands**: `create` or `extract`
- NOT `archive-create` / `archive-extract`
- Create: files first, then `-o` output
- Compression: zlib, bzip2, lzma, none (lowercase)

### ✅ Compress
```
filevault compress <input> -o <output> -a <algorithm> -l <level>
```
- Algorithm: zlib, bzip2, lzma (lowercase)
- Level: 1-9
- Must use `-o` for output

## Common Issues Fixed

1. **Algorithm names must be lowercase**: RSA-4096 → rsa-4096
2. **KDF names must be lowercase**: Argon2id → argon2id
3. **Format parameter**: Hash uses `--format` not `-f`
4. **Subcommands**: stego and archive use subcommands (embed/extract, create/extract)
5. **Keygen**: No separate key-size parameter, no password support
6. **Sign/Verify**: Positional arguments, not flags
7. **Compression**: Algorithm values lowercase (LZMA → lzma)

## Test Commands

```bash
# Encrypt
filevault encrypt input.txt output.enc -a aes-256-gcm -p password123 -m standard --kdf argon2id --compression lzma

# Decrypt
filevault decrypt output.enc decrypted.txt -p password123

# Hash
filevault hash input.txt -a sha256 --format hex

# Keygen
filevault keygen -a rsa-4096 -o mykey
# Creates: mykey.pub and mykey.key

# Sign
filevault sign document.txt mykey.key -o document.sig

# Verify
filevault verify document.txt document.sig mykey.pub

# Stego
filevault stego embed secret.txt cover.png output.png
filevault stego extract output.png extracted.txt

# Archive
filevault archive create file1.txt file2.txt -o backup.fva -c lzma -p password
filevault archive extract backup.fva -o extracted/ -p password

# Compress
filevault compress large.txt -o compressed.lzma -a lzma -l 9
filevault compress compressed.lzma -o decompressed.txt -d
```

## All Fixed!

Hot reload should apply changes. Test each command in GUI now.
