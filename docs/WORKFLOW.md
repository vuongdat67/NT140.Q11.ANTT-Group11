# FileVault Workflow & Architecture

Comprehensive guide to FileVault's internal workflows, encryption process, and system architecture.

## Table of Contents
- [Encryption Workflow](#encryption-workflow)
- [Decryption Workflow](#decryption-workflow)
- [Key Derivation](#key-derivation)
- [File Format](#file-format)
- [Security Considerations](#security-considerations)
- [Algorithm Selection Guide](#algorithm-selection-guide)

---

## Encryption Workflow

### High-Level Flow

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐    ┌────────────┐
│ Input File  │───▶│  Compression │───▶│  Encryption │───▶│ Output File│
└─────────────┘    │  (optional)  │    │  (AEAD/PQC) │    │  (.fvlt)   │
                   └──────────────┘    └─────────────┘    └────────────┘
                          │                   │
                          ▼                   ▼
                   ┌──────────────┐    ┌─────────────┐
                   │ ZLIB/LZMA/   │    │ KDF (Argon2)│
                   │ BZIP2        │    │ + Salt/Nonce│
                   └──────────────┘    └─────────────┘
```

### Detailed Process

#### 1. Input Validation
```
- Check file exists and is readable
- Verify file size (streaming for large files)
- Validate algorithm selection
```

#### 2. Key Material Generation
```
Password-based:
  ┌──────────┐    ┌─────────────┐    ┌─────────────┐
  │ Password │───▶│ KDF (Argon2)│───▶│ 256-bit Key │
  └──────────┘    │ + Salt      │    └─────────────┘
                  └─────────────┘
                        ▲
                        │
                  ┌─────┴─────┐
                  │ Random    │
                  │ Salt (32B)│
                  └───────────┘

Asymmetric (RSA/ECC):
  ┌────────────┐    ┌─────────────┐    ┌─────────────┐
  │ Public Key │───▶│ Key Wrap    │───▶│ Encrypted   │
  └────────────┘    │ (RSA-OAEP)  │    │ Session Key │
                    └─────────────┘    └─────────────┘

Post-Quantum (Kyber):
  ┌────────────┐    ┌─────────────┐    ┌─────────────┐
  │ Public Key │───▶│ KEM Encaps  │───▶│ Shared      │
  └────────────┘    │ (Kyber)     │    │ Secret      │
                    └─────────────┘    └──────┬──────┘
                                              │
                                              ▼
                                       ┌─────────────┐
                                       │ HKDF ──▶ Key│
                                       └─────────────┘
```

#### 3. Compression (Optional)
```
┌───────────┐    ┌─────────────────────────────────────────┐
│ Plaintext │───▶│ Compressor (ZLIB/LZMA/BZIP2)            │
└───────────┘    │                                         │
                 │ - Level 1-9 (default: 6)               │
                 │ - Ratio depends on file type            │
                 │   - Text: 60-80% reduction              │
                 │   - Binary: 10-40% reduction            │
                 │   - Already compressed: minimal         │
                 └─────────────────────────────────────────┘
```

#### 4. Encryption

**AEAD Mode (Recommended):**
```
                   ┌─────────────────┐
                   │ Authentication  │
            ┌─────▶│ Tag (16 bytes)  │
            │      └─────────────────┘
            │
┌───────────┴───────────┐
│    AEAD Cipher        │
│  (AES-GCM, ChaCha20)  │
└───────────┬───────────┘
            │
            ├─────▶ Ciphertext
            │
┌───────────┴───────────┐
│ Associated Data (AAD) │
│ - Header hash         │
│ - Metadata            │
└───────────────────────┘
```

**Block Cipher Modes (Legacy):**
```
┌───────────┐    ┌─────────────┐    ┌────────────┐
│ Plaintext │───▶│ Padding     │───▶│ Block Mode │
└───────────┘    │ (PKCS7)     │    │ (CBC/CTR)  │
                 └─────────────┘    └────────────┘
                                          │
                                          ▼
                                    ┌────────────┐
                                    │ HMAC-SHA256│ (for authentication)
                                    └────────────┘
```

#### 5. File Format Assembly
```
┌────────────────────────────────────────────────────────────┐
│                    FileVault Format (.fvlt)                │
├────────────────────────────────────────────────────────────┤
│ Magic (4B) │ Version (2B) │ Flags (2B) │ Header Length (4B)│
├────────────────────────────────────────────────────────────┤
│                      Header (variable)                      │
│ - Algorithm ID                                              │
│ - KDF Parameters                                            │
│ - Salt (32 bytes)                                           │
│ - Nonce/IV (12-16 bytes)                                   │
│ - Compression info                                          │
│ - Original filename                                         │
├────────────────────────────────────────────────────────────┤
│                    Ciphertext (variable)                    │
├────────────────────────────────────────────────────────────┤
│                  Auth Tag (16 bytes)                        │
└────────────────────────────────────────────────────────────┘
```

---

## Decryption Workflow

### Process

```
┌────────────┐    ┌──────────────┐    ┌───────────────┐    ┌────────────┐
│ Input File │───▶│ Parse Header │───▶│ Derive Key    │───▶│ Decrypt    │
│  (.fvlt)   │    │              │    │ (KDF/KEM)     │    │            │
└────────────┘    └──────────────┘    └───────────────┘    └─────┬──────┘
                         │                                        │
                         ▼                                        ▼
                  ┌──────────────┐                         ┌────────────┐
                  │ Validate     │                         │ Verify Tag │
                  │ - Magic      │                         └──────┬─────┘
                  │ - Version    │                                │
                  │ - Checksum   │                                ▼
                  └──────────────┘                         ┌────────────┐
                                                           │ Decompress │
                                                           │ (if needed)│
                                                           └──────┬─────┘
                                                                  │
                                                                  ▼
                                                           ┌────────────┐
                                                           │ Output File│
                                                           └────────────┘
```

### Error Handling

| Error | Cause | Solution |
|-------|-------|----------|
| Invalid magic | Not a FileVault file | Check file source |
| Version mismatch | Newer format | Upgrade FileVault |
| Auth tag mismatch | Wrong password / tampering | Verify password |
| KDF failure | Insufficient memory | Reduce security level |
| Decompression error | Corrupted data | File may be damaged |

---

## Key Derivation

### Argon2id (Recommended)

**Why Argon2id?**
- Winner of Password Hashing Competition (2015)
- Resistant to GPU/ASIC attacks
- Hybrid of Argon2i (side-channel resistant) and Argon2d (memory-hard)

**Parameters by Security Level:**

| Level | Time Cost | Memory | Parallelism | Time |
|-------|-----------|--------|-------------|------|
| weak | 1 | 4 MB | 1 | ~2ms |
| medium | 2 | 16 MB | 2 | ~10ms |
| strong | 3 | 64 MB | 4 | ~30ms |
| paranoid | 4 | 128 MB | 4 | ~60ms |

**Process:**
```
                    ┌─────────────────┐
                    │    Password     │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │     Argon2id    │
                    │                 │
                    │ t = time_cost   │
                    │ m = memory_cost │
                    │ p = parallelism │
                    └────────┬────────┘
                             │
             ┌───────────────┼───────────────┐
             │               │               │
             ▼               ▼               ▼
      ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
      │ Encryption  │ │ Auth Tag    │ │ Header      │
      │ Key (256b)  │ │ Key (256b)  │ │ Key (256b)  │
      └─────────────┘ └─────────────┘ └─────────────┘
```

---

## File Format

### Header Structure (v1.0)

```cpp
struct FileHeader {
    uint8_t  magic[4];         // "FV\x01\x00"
    uint16_t version;          // 0x0100
    uint16_t flags;            // Compression, etc.
    uint32_t header_length;    // Total header size
    uint8_t  algorithm_id;     // Algorithm enum
    uint8_t  kdf_id;           // KDF enum
    uint8_t  compression_id;   // Compression enum
    uint8_t  reserved;         // Future use
    uint32_t kdf_time_cost;    // Argon2 t parameter
    uint32_t kdf_memory_cost;  // Argon2 m parameter (KB)
    uint8_t  kdf_parallelism;  // Argon2 p parameter
    uint8_t  salt[32];         // Random salt
    uint8_t  nonce[16];        // IV/Nonce (12-16 bytes)
    uint64_t original_size;    // Uncompressed size
    // ... additional fields
};
```

### Versioning

| Version | Features |
|---------|----------|
| 1.0 | Initial format, AEAD, compression |
| 1.1 | (planned) PQC support, metadata encryption |
| 2.0 | (planned) Streaming format, partial decryption |

---

## Security Considerations

### Nonce Management

```
Every encryption generates:
- Unique nonce (12-16 bytes from CSPRNG)
- Never reused with same key
- Stored in header

FileVault guarantees:
- 96-bit nonce for GCM (standard)
- 192-bit nonce for XChaCha20 (optional)
- Collision probability: 2^-48 after 2^24 files
```

### Key Separation

```
Master Password
      │
      ▼
   Argon2id
      │
      ├──▶ Encryption Key (unique per file)
      ├──▶ Auth Key (for HMAC in non-AEAD)
      └──▶ Header Key (metadata protection)
```

### Threat Model

| Threat | Protection |
|--------|------------|
| Passive eavesdropping | AEAD encryption |
| Active tampering | Authentication tag |
| Brute-force password | Argon2id, time/memory cost |
| Known-plaintext | Semantic security of ciphers |
| Quantum computers | Kyber-Hybrid option |
| Side-channel | Constant-time implementations |

---

## Algorithm Selection Guide

### Decision Tree

```
                         ┌──────────────────┐
                         │ What's your need?│
                         └────────┬─────────┘
                                  │
           ┌──────────────────────┼──────────────────────┐
           │                      │                      │
           ▼                      ▼                      ▼
     ┌─────────┐           ┌─────────────┐        ┌───────────┐
     │ General │           │ Quantum-    │        │ Legacy    │
     │ Purpose │           │ Resistant   │        │ Compat    │
     └────┬────┘           └──────┬──────┘        └─────┬─────┘
          │                       │                     │
          ▼                       ▼                     ▼
   ┌─────────────┐         ┌────────────┐        ┌───────────┐
   │ AES-256-GCM │         │ Kyber-1024 │        │ AES-256-  │
   │ (default)   │         │ -Hybrid    │        │ CBC/CTR   │
   └─────────────┘         └────────────┘        └───────────┘
```

### Recommendations by Use Case

| Use Case | Recommended Algorithm | Security Level |
|----------|----------------------|----------------|
| Personal files | AES-256-GCM | medium |
| Business documents | AES-256-GCM | strong |
| Long-term archive | Kyber-1024-Hybrid | paranoid |
| Mobile/IoT | ChaCha20-Poly1305 | medium |
| Disk encryption | AES-256-XTS | strong |
| Compliance (NIST) | AES-256-GCM | strong |
| Compliance (Korea) | ARIA-256-GCM | strong |
| Compliance (Japan) | Camellia-256-GCM | strong |
| Compliance (China) | SM4-GCM | medium |

---

## Streaming Encryption

For large files (>1GB), FileVault uses streaming mode:

```
┌────────────────────────────────────────────────────────────┐
│                   Streaming Architecture                    │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  ┌───────┐     ┌───────┐     ┌───────┐     ┌───────┐      │
│  │ Chunk │────▶│ Chunk │────▶│ Chunk │────▶│ Chunk │      │
│  │  1    │     │  2    │     │  3    │     │  N    │      │
│  └───────┘     └───────┘     └───────┘     └───────┘      │
│      │             │             │             │           │
│      ▼             ▼             ▼             ▼           │
│  ┌───────┐     ┌───────┐     ┌───────┐     ┌───────┐      │
│  │Encrypt│     │Encrypt│     │Encrypt│     │Encrypt│      │
│  │+Auth  │     │+Auth  │     │+Auth  │     │+Auth  │      │
│  └───────┘     └───────┘     └───────┘     └───────┘      │
│                                                            │
│  Chunk size: Adaptive (1MB - 64MB based on available RAM)  │
│  Each chunk has unique derived nonce                       │
│  Final chunk includes total chunk count for verification   │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

**Benefits:**
- Constant memory usage (~2x chunk size)
- Progress reporting
- Resumable (future feature)
- Parallel encryption (future feature)

---

## Performance Optimization

### Hardware Acceleration

```
┌───────────────────────────────────────────────────────────┐
│                 CPU Feature Detection                      │
├───────────────────────────────────────────────────────────┤
│                                                           │
│  AES-NI detected? ───▶ Yes ───▶ Use AES-GCM (700+ MB/s)  │
│         │                                                 │
│         └──▶ No ───▶ Use ChaCha20-Poly1305 (600+ MB/s)   │
│                                                           │
│  ARM Crypto? ───▶ Yes ───▶ Use AES-GCM (ARMv8 accel)     │
│         │                                                 │
│         └──▶ No ───▶ Use ChaCha20-Poly1305 (optimized)   │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

### Memory Management

| File Size | Strategy | Memory Usage |
|-----------|----------|--------------|
| < 10 MB | In-memory | File size × 2 |
| 10 MB - 1 GB | Buffered streaming | 64 MB |
| > 1 GB | Chunked streaming | 64-128 MB |

---

## Error Recovery

### Corrupted File Handling

```
┌─────────────────┐
│ Read Header     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐     ┌─────────────────┐
│ Valid Header?   │──No─▶│ Report Error   │
└────────┬────────┘     │ Invalid format  │
         │ Yes          └─────────────────┘
         ▼
┌─────────────────┐     ┌─────────────────┐
│ Decrypt Chunk   │──Error─▶│ Try Next Chunk│
└────────┬────────┘     │ (partial recovery)│
         │ Success      └─────────────────┘
         ▼
┌─────────────────┐
│ Verify Auth Tag │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
   OK       Fail
    │         │
    ▼         ▼
┌──────┐  ┌───────────────────┐
│Output│  │Wrong password or  │
│ OK   │  │data tampered      │
└──────┘  └───────────────────┘
```

---

## References

- [NIST SP 800-38D: AES-GCM](https://csrc.nist.gov/publications/detail/sp/800-38d/final)
- [RFC 8439: ChaCha20-Poly1305](https://datatracker.ietf.org/doc/html/rfc8439)
- [NIST FIPS 203: ML-KEM (Kyber)](https://csrc.nist.gov/pubs/fips/203/final)
- [NIST FIPS 204: ML-DSA (Dilithium)](https://csrc.nist.gov/pubs/fips/204/final)
- [Password Hashing Competition: Argon2](https://password-hashing.net/)
- [Botan Cryptographic Library](https://botan.randombit.net/)
