# 📚 FileVault - Tài liệu Thuật toán Mật mã

> Tài liệu chi tiết về tất cả các thuật toán mật mã được sử dụng trong FileVault.
> Dành cho **học tập**, **nghiên cứu** và **phát triển**.

## 📖 Mục lục

### 🔐 [Symmetric - Mã hóa Đối xứng](./symmetric/)

Thuật toán sử dụng **cùng một khóa** cho cả mã hóa và giải mã.

| Thuật toán | Loại | Bảo mật | File |
|------------|------|---------|------|
| **AES-GCM** | AEAD | ⭐⭐⭐ Maximum | [aes-gcm.md](./symmetric/aes-gcm.md) |
| **AES-CBC** | Block | ⭐⭐ Strong | [aes-cbc.md](./symmetric/aes-cbc.md) |
| **AES-CTR** | Stream | ⭐⭐ Strong | [aes-ctr.md](./symmetric/aes-ctr.md) |
| **AES-XTS** | Disk | ⭐⭐⭐ Maximum | [aes-xts.md](./symmetric/aes-xts.md) |
| **ChaCha20-Poly1305** | AEAD | ⭐⭐⭐ Maximum | [chacha20-poly1305.md](./symmetric/chacha20-poly1305.md) |
| **Serpent-GCM** | AEAD | ⭐⭐⭐ Maximum | [serpent-gcm.md](./symmetric/serpent-gcm.md) |
| **Twofish-GCM** | AEAD | ⭐⭐⭐ Maximum | [twofish-gcm.md](./symmetric/twofish-gcm.md) |
| **Camellia-GCM** | AEAD | ⭐⭐⭐ Maximum | [camellia-gcm.md](./symmetric/camellia-gcm.md) |
| **ARIA-GCM** | AEAD | ⭐⭐⭐ Maximum | [aria-gcm.md](./symmetric/aria-gcm.md) |
| **SM4-GCM** | AEAD | ⭐⭐ Strong | [sm4-gcm.md](./symmetric/sm4-gcm.md) |
| **3DES** | Block | ⚠️ Legacy | [triple-des.md](./symmetric/triple-des.md) |

### 🔑 [Asymmetric - Mã hóa Bất đối xứng](./asymmetric/)

Thuật toán sử dụng **cặp khóa** public/private.

| Thuật toán | Loại | Bảo mật | File |
|------------|------|---------|------|
| **RSA-OAEP** | Encryption | ⭐⭐⭐ Maximum | [rsa.md](./asymmetric/rsa.md) |
| **ECC (ECDH)** | Key Exchange | ⭐⭐⭐ Maximum | [ecc.md](./asymmetric/ecc.md) |

### 🛡️ [PQC - Post-Quantum Cryptography](./pqc/)

Thuật toán **chống lại máy tính lượng tử** (NIST Selected).

| Thuật toán | Loại | NIST Level | File |
|------------|------|------------|------|
| **Kyber** (ML-KEM) | KEM | Level 1-5 | [kyber.md](./pqc/kyber.md) |
| **Dilithium** (ML-DSA) | Signature | Level 2-5 | [dilithium.md](./pqc/dilithium.md) |

### 📚 [Classical - Mật mã Cổ điển](./classical/)

Thuật toán **giáo dục** - KHÔNG an toàn cho sử dụng thực tế!

| Thuật toán | Loại | File |
|------------|------|------|
| **Caesar** | Shift | [caesar.md](./classical/caesar.md) |
| **Vigenère** | Polyalphabetic | [vigenere.md](./classical/vigenere.md) |
| **Playfair** | Digraph | [playfair.md](./classical/playfair.md) |
| **Hill** | Matrix | [hill.md](./classical/hill.md) |
| **Substitution** | Monoalphabetic | [substitution.md](./classical/substitution.md) |

### 🔒 [KDF - Key Derivation Functions](./kdf/)

Hàm dẫn xuất khóa từ mật khẩu.

| Thuật toán | Loại | File |
|------------|------|------|
| **Argon2id** | Memory-hard | [argon2.md](./kdf/argon2.md) |
| **PBKDF2** | Standard | [pbkdf2.md](./kdf/pbkdf2.md) |
| **HKDF** | Expand | [hkdf.md](./kdf/hkdf.md) |

### #️⃣ [Hash - Hàm băm](./hash/)

| Thuật toán | Output | File |
|------------|--------|------|
| **SHA-256/512** | 256/512-bit | [sha2.md](./hash/sha2.md) |
| **SHA3** | 256/512-bit | [sha3.md](./hash/sha3.md) |
| **BLAKE2** | Variable | [blake2.md](./hash/blake2.md) |

---

## 🎯 Cách đọc tài liệu

Mỗi file thuật toán có cấu trúc:

1. **Tóm tắt** - Giới thiệu ngắn gọn
2. **Lý thuyết** - Nền tảng toán học
3. **Cách hoạt động** - Step-by-step với diagram
4. **Cấu trúc dữ liệu** - Key size, IV, block size
5. **Implement trong FileVault** - Code thực tế
6. **Lỗ hổng & Mitigation** - Bảo mật
7. **Test Vectors** - Kiểm tra tính đúng đắn
8. **Ví dụ sử dụng** - CLI commands
9. **Tham khảo** - Links, RFCs

---

## 🔧 Build & Test

```bash
# Chạy tất cả tests
ctest --test-dir build/build/Release -j16

# Test cụ thể một thuật toán
filevault benchmark --symmetric   # Test symmetric
filevault benchmark --pqc         # Test post-quantum
```

---

## 📊 So sánh nhanh

### AEAD Algorithms (Khuyến nghị cho mã hóa file)

```
┌─────────────────────┬──────────┬───────────┬────────┬─────────────┐
│     Algorithm       │ Key Size │ Nonce/IV  │  Tag   │   Speed     │
├─────────────────────┼──────────┼───────────┼────────┼─────────────┤
│ AES-256-GCM         │ 256-bit  │ 96-bit    │ 128-bit│ ████████░░  │
│ ChaCha20-Poly1305   │ 256-bit  │ 96-bit    │ 128-bit│ ██████████  │
│ Serpent-256-GCM     │ 256-bit  │ 96-bit    │ 128-bit│ █████░░░░░  │
└─────────────────────┴──────────┴───────────┴────────┴─────────────┘
```

### Security Comparison

```
                    Classical   AES    ChaCha20   PQC (Kyber)
                    ─────────   ───    ────────   ───────────
Brute Force         Minutes     2^256  2^256      2^256
Quantum (Grover)    N/A         2^128  2^128      Still 2^256
Quantum (Shor)      N/A         N/A    N/A        Safe ✓
```

---

**FileVault** - Learn cryptography by doing! 🔐
