# FileVault Architecture & Workflow

Technical documentation for FileVault's internal architecture, data flow, and operational workflows.

## Table of Contents
- [System Architecture](#system-architecture)
- [Core Components](#core-components)
- [Encryption Workflow](#encryption-workflow)
- [Decryption Workflow](#decryption-workflow)
- [File Format](#file-format)
- [Security Design](#security-design)
- [Performance Considerations](#performance-considerations)

---

## System Architecture

### High-Level Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         CLI Layer                           │
│  (Command Parsing, User Interaction, Progress Display)     │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│                    Command Layer                            │
│  encrypt_cmd │ decrypt_cmd │ keygen_cmd │ benchmark_cmd    │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│                   Core Engine                               │
│         CryptoEngine (Algorithm Registry)                   │
└──────┬──────────────┬──────────────┬───────────────────────┘
       │              │              │
┌──────▼──────┐ ┌─────▼──────┐ ┌────▼─────────┐
│  Symmetric  │ │ Asymmetric │ │     PQC      │
│  Algorithms │ │ Algorithms │ │  Algorithms  │
└──────┬──────┘ └─────┬──────┘ └────┬─────────┘
       │              │              │
┌──────▼──────────────▼──────────────▼─────────────┐
│               Botan 3.x Library                   │
│  (Cryptographic Primitives, RNG, Key Management) │
└──────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Responsibility |
|-----------|---------------|
| **CLI Layer** | Parse arguments, validate inputs, display progress |
| **Command Layer** | Implement business logic for each command |
| **Core Engine** | Algorithm registration, key derivation, streaming |
| **Crypto Algorithms** | Wrapper classes for Botan primitives |
| **File I/O Utils** | Memory-mapped I/O, buffered reading/writing |
| **Progress Utils** | Real-time progress bars, ETA calculation |

---

## Core Components

### 1. CryptoEngine (`core/crypto_engine.cpp`)

**Purpose:** Central registry and factory for all cryptographic operations.

**Key Methods:**
```cpp
class CryptoEngine {
    // Algorithm registration
    void register_algorithm(std::unique_ptr<CryptoAlgorithm> algo);
    
    // Algorithm lookup
    CryptoAlgorithm* get_algorithm(const std::string& name);
    
    // Key derivation
    Result<std::vector<uint8_t>> derive_key(
        const std::string& password,
        const std::vector<uint8_t>& salt,
        const KDFParams& params
    );
    
    // File operations
    Result<void> encrypt_file(const EncryptParams& params);
    Result<void> decrypt_file(const DecryptParams& params);
};
```

**Initialization Flow:**
1. Create CryptoEngine instance
2. Register all symmetric algorithms (AES, ChaCha20, etc.)
3. Register asymmetric algorithms (RSA, ECC)
4. Register PQC algorithms (Kyber, Dilithium)
5. Set default algorithm (AES-256-GCM)

---

### 2. CryptoAlgorithm Interface (`core/crypto_algorithm.hpp`)

**Purpose:** Abstract interface for all encryption algorithms.

```cpp
class CryptoAlgorithm {
public:
    virtual ~CryptoAlgorithm() = default;
    
    // Metadata
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual size_t key_size() const = 0;
    virtual size_t iv_size() const = 0;
    virtual bool is_aead() const = 0;
    
    // Encryption
    virtual Result<std::vector<uint8_t>> encrypt(
        const std::vector<uint8_t>& plaintext,
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& iv
    ) = 0;
    
    // Decryption
    virtual Result<std::vector<uint8_t>> decrypt(
        const std::vector<uint8_t>& ciphertext,
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& iv
    ) = 0;
};
```

**Implementations:**
- **AEAD Ciphers:** `AES_GCM`, `ChaCha20Poly1305`, `Serpent_GCM`
- **Block Modes:** `AES_CBC`, `AES_CTR`, `AES_XTS`
- **Asymmetric:** `RSA`, `ECCHybrid`
- **PQC:** `KyberHybrid`, `KyberKEM`, `Dilithium`

---

### 3. Streaming Engine (`core/streaming.cpp`)

**Purpose:** Memory-efficient encryption/decryption for large files.

**Key Features:**
- **Adaptive Chunking:** Adjusts chunk size based on available RAM
- **Zero-Copy I/O:** Memory-mapped files when beneficial
- **Progress Tracking:** Real-time updates with ETA
- **Error Recovery:** Atomic operations with rollback

**Memory Management:**
```cpp
class StreamingEngine {
    // Calculate optimal chunk size
    size_t calculate_chunk_size() {
        size_t available_memory = get_available_memory();
        size_t file_size = get_file_size(input_path);
        
        // Use 10% of available RAM, min 1MB, max 64MB
        size_t chunk = std::clamp(
            available_memory / 10,
            1ULL << 20,   // 1 MB
            64ULL << 20   // 64 MB
        );
        
        return chunk;
    }
};
```

**Processing Flow:**
1. Open input file (read mode)
2. Create output file (write mode)
3. Calculate optimal chunk size
4. Initialize progress bar
5. Process chunks:
   - Read chunk from input
   - Encrypt/decrypt chunk
   - Write to output
   - Update progress
6. Finalize and verify

---

### 4. File Format (`format/file_header.cpp`)

**Purpose:** Standardized format for encrypted files (`.fvlt`).

**Structure:**
```
┌─────────────────────────────────────────────────────┐
│                  File Header                        │
├─────────────────────────────────────────────────────┤
│ Magic Number (8 bytes): "FILEVLT\x00"              │
│ Version (2 bytes): 0x0100 (v1.0)                   │
│ Algorithm ID (2 bytes): Enum value                 │
│ KDF Type (1 byte): Argon2id/PBKDF2/Scrypt          │
│ Security Level (1 byte): weak/medium/strong        │
│ Flags (2 bytes): Compression, PQC, etc.            │
├─────────────────────────────────────────────────────┤
│ Salt (32 bytes): Random salt for KDF               │
│ Nonce/IV (12-16 bytes): Algorithm-specific         │
│ KDF Parameters (variable): Iterations, memory       │
├─────────────────────────────────────────────────────┤
│ Original Filename Length (2 bytes)                  │
│ Original Filename (UTF-8)                           │
├─────────────────────────────────────────────────────┤
│ Metadata Length (4 bytes)                           │
│ Metadata (JSON): timestamps, compression, etc.     │
├─────────────────────────────────────────────────────┤
│ Header MAC (32 bytes): HMAC-SHA256 of header       │
├─────────────────────────────────────────────────────┤
│                                                     │
│              Encrypted Payload                      │
│                                                     │
├─────────────────────────────────────────────────────┤
│ Authentication Tag (16 bytes): For AEAD modes      │
└─────────────────────────────────────────────────────┘
```

**Header Fields:**

| Field | Size | Description |
|-------|------|-------------|
| Magic Number | 8 bytes | "FILEVLT\x00" identifier |
| Version | 2 bytes | Major.Minor (0x0100 = 1.0) |
| Algorithm ID | 2 bytes | Algorithm enum value |
| KDF Type | 1 byte | 0=Argon2id, 1=PBKDF2, 2=Scrypt |
| Security Level | 1 byte | 0=weak, 1=medium, 2=strong, 3=paranoid |
| Flags | 2 bytes | Bit flags for features |
| Salt | 32 bytes | Random salt for key derivation |
| Nonce/IV | 12-16 bytes | Algorithm-specific initialization vector |
| KDF Params | Variable | Iterations, memory cost, parallelism |
| Filename | Variable | UTF-8 encoded original filename |
| Metadata | Variable | JSON encoded metadata |
| Header MAC | 32 bytes | HMAC-SHA256 for header integrity |

**Flags Bitfield:**
```
Bit 0: Compression enabled
Bit 1: PQC algorithm used
Bit 2: Asymmetric encryption
Bit 3: Steganography applied
Bit 4-15: Reserved
```

---

## Encryption Workflow

### Password-Based Encryption (Symmetric)

```
┌─────────────┐
│ User Input  │
│ - File      │
│ - Password  │
│ - Algorithm │
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 1. Generate Random Salt (32 bytes)  │
│    crypto::random_bytes(32)         │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 2. Derive Key from Password         │
│    Argon2id(password, salt, params) │
│    → 256-bit key                    │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 3. Generate Random Nonce (12 bytes) │
│    crypto::random_bytes(12)         │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 4. Create File Header               │
│    - Magic, version, algorithm ID   │
│    - Salt, nonce, KDF params        │
│    - Original filename, metadata    │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 5. Compute Header MAC               │
│    HMAC-SHA256(header, key)         │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 6. Write Header to Output File      │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 7. Encrypt File in Chunks           │
│    FOR each chunk:                  │
│      - Read plaintext chunk         │
│      - Encrypt with AES-GCM         │
│      - Write ciphertext chunk       │
│      - Update progress bar          │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 8. Write Authentication Tag         │
│    (For AEAD modes like GCM)        │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 9. Finalize and Close Files         │
│    - Sync to disk                   │
│    - Verify output file size        │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────┐
│  Success!   │
│ .fvlt file  │
└─────────────┘
```

### Public Key Encryption (Asymmetric)

#### RSA Encryption
```
1. Load RSA public key from PEM file
2. Generate random AES-256 key (32 bytes)
3. Encrypt file with AES-256-GCM:
   - Derive key and nonce
   - Encrypt payload
4. Encrypt AES key with RSA public key:
   - RSA-OAEP encryption
   - Store encrypted key in header
5. Write header + encrypted payload
```

#### ECC Hybrid Encryption
```
1. Load ECC public key
2. Generate ephemeral ECC keypair
3. Perform ECDH key agreement:
   - shared_secret = ECDH(ephemeral_private, recipient_public)
4. Derive AES key from shared secret:
   - key = KDF(shared_secret, salt)
5. Encrypt file with AES-256-GCM
6. Include ephemeral public key in header
```

#### Kyber-Hybrid Encryption (PQC)
```
1. Load Kyber-1024 public key
2. Perform KEM encapsulation:
   - (ciphertext, shared_secret) = Kyber.Encapsulate(public_key)
3. Derive AES key from shared secret:
   - key = HKDF(shared_secret, salt, "FileVault")
4. Encrypt file with AES-256-GCM
5. Store KEM ciphertext in header
6. Provides quantum resistance!
```

---

## Decryption Workflow

### Password-Based Decryption

```
┌─────────────┐
│ User Input  │
│ - .fvlt     │
│ - Password  │
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 1. Read and Parse File Header       │
│    - Verify magic number            │
│    - Extract algorithm ID, KDF type │
│    - Read salt, nonce, params       │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 2. Derive Key from Password         │
│    Use same KDF with stored params: │
│    Argon2id(password, salt, params) │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 3. Verify Header MAC                │
│    Compute HMAC-SHA256(header, key) │
│    Compare with stored MAC          │
│    → If mismatch: Wrong password!   │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 4. Initialize Decryption Cipher     │
│    AES-GCM with derived key + nonce │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 5. Decrypt File in Chunks           │
│    FOR each chunk:                  │
│      - Read ciphertext chunk        │
│      - Decrypt with AES-GCM         │
│      - Write plaintext chunk        │
│      - Update progress bar          │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 6. Verify Authentication Tag        │
│    (For AEAD modes)                 │
│    → If invalid: File tampered!     │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────┐
│ 7. Restore Original Filename        │
│    Use filename from header         │
└──────┬──────────────────────────────┘
       │
       ▼
┌─────────────┐
│  Success!   │
│ Decrypted   │
│    file     │
└─────────────┘
```

### Public Key Decryption

#### RSA Decryption
```
1. Read header and extract encrypted AES key
2. Decrypt AES key with RSA private key:
   - plaintext_key = RSA_Decrypt(encrypted_key, private_key)
3. Decrypt file with recovered AES key
4. Verify authentication tag
```

#### ECC Decryption
```
1. Extract ephemeral public key from header
2. Load recipient's private key
3. Perform ECDH:
   - shared_secret = ECDH(recipient_private, ephemeral_public)
4. Derive AES key from shared secret
5. Decrypt file with AES-256-GCM
```

#### Kyber-Hybrid Decryption
```
1. Read KEM ciphertext from header
2. Perform KEM decapsulation:
   - shared_secret = Kyber.Decapsulate(ciphertext, private_key)
3. Derive AES key from shared secret
4. Decrypt file with AES-256-GCM
5. Quantum-resistant decryption complete!
```

---

## Security Design

### Defense-in-Depth

**Layer 1: Algorithm Security**
- Use only well-vetted, NIST-approved algorithms
- AEAD ciphers for authenticated encryption
- Post-quantum algorithms for future-proofing

**Layer 2: Key Management**
- Strong KDF (Argon2id) with tunable parameters
- Random salt per encryption (prevents rainbow tables)
- Secure key derivation from passwords

**Layer 3: Implementation Security**
- Use Botan (audited crypto library)
- Constant-time operations where critical
- Secure memory wiping (zero key material)

**Layer 4: File Integrity**
- Header MAC prevents tampering
- AEAD authentication tag for payload
- Version field allows format migration

**Layer 5: Operational Security**
- Nonce uniqueness enforced
- Atomic file operations (no partial writes)
- Progress indication without leaking data

### Threat Model

**Protected Against:**
- ✅ Brute-force password attacks (strong KDF)
- ✅ Dictionary attacks (salt + iterations)
- ✅ Chosen-ciphertext attacks (AEAD)
- ✅ Tampering (MAC + auth tag)
- ✅ Traffic analysis (encrypted metadata)
- ✅ Quantum attacks (PQC algorithms)

**Not Protected Against:**
- ❌ Weak passwords (use strong passwords!)
- ❌ Keyloggers/malware (OS-level threat)
- ❌ Side-channel attacks (hardware-level)
- ❌ Coercion (rubber-hose cryptanalysis)

### Randomness

**Sources:**
- Primary: OS CSPRNG (`/dev/urandom`, `CryptGenRandom`)
- Library: Botan's AutoSeeded_RNG (entropy pooling)

**Usage:**
- Salt generation (32 bytes per encryption)
- Nonce/IV generation (12-16 bytes)
- Key generation (asymmetric keypairs)
- Session keys (hybrid encryption)

**Quality Assurance:**
- DRBG health checks
- Entropy accumulation
- Reseeding after fork (Unix)

---

## Performance Considerations

### Bottlenecks

1. **I/O Operations** (Usually the slowest)
   - Solution: Buffered I/O, memory mapping
   
2. **Key Derivation** (Intentionally slow)
   - Solution: Cache derived keys (carefully)
   
3. **Encryption/Decryption** (CPU-bound)
   - Solution: Hardware acceleration (AES-NI)

### Optimizations

**1. Chunked Processing**
```cpp
// Adaptive chunk size based on file size and RAM
size_t chunk_size = calculate_optimal_chunk_size(file_size);

// Process in parallel (future enhancement)
#pragma omp parallel for
for (size_t i = 0; i < num_chunks; ++i) {
    process_chunk(i);
}
```

**2. Hardware Acceleration**
- AES-NI instructions for AES-GCM
- AVX2/AVX512 for ChaCha20
- Compiler optimizations (`-O3 -march=native`)

**3. Memory Management**
- Stack allocation for small buffers
- Arena allocators for frequent allocations
- Secure wiping on deallocation

**4. Progress Feedback**
- Non-blocking progress updates
- Minimal overhead (<1% CPU)
- Accurate ETA calculation

### Benchmarks

**Test Environment:** Intel i7-11800H, 32GB RAM, NVMe SSD

| Operation | Throughput | Notes |
|-----------|-----------|-------|
| AES-256-GCM Encrypt | ~700 MB/s | Hardware accelerated |
| ChaCha20-Poly1305 | ~600 MB/s | Software optimized |
| Kyber-1024-Hybrid | ~650 MB/s | Minimal overhead |
| Argon2id (medium) | ~10ms | Per key derivation |
| RSA-4096 Keygen | ~1.7s | One-time cost |
| Kyber-1024 Keygen | ~0.4ms | Fast PQC! |

---

## Error Handling

### Result Type

```cpp
template<typename T>
class Result {
    std::variant<T, Error> value_;
    
public:
    bool is_ok() const;
    bool is_err() const;
    T unwrap();
    Error error();
};
```

**Usage:**
```cpp
auto result = encrypt_file(params);
if (result.is_err()) {
    console::error("Encryption failed: {}", result.error().message());
    return 1;
}
```

### Error Categories

1. **I/O Errors:** File not found, permission denied
2. **Crypto Errors:** Wrong password, corrupted file
3. **Format Errors:** Invalid header, unsupported version
4. **System Errors:** Out of memory, disk full

---

## Future Enhancements

### Planned Features

1. **Parallel Processing**
   - Multi-threaded chunk encryption
   - SIMD optimizations
   
2. **Cloud Integration**
   - S3/Azure Blob Storage support
   - Streaming encryption to cloud
   
3. **Key Management**
   - Hardware security module (HSM) support
   - TPM integration for key protection
   
4. **Advanced Compression**
   - Context-aware compression
   - Encryption + compression pipeline
   
5. **Metadata Encryption**
   - Hide file size (padding)
   - Encrypted filenames

---

## References

- [Botan Crypto Library](https://botan.randombit.net/)
- [NIST Cryptographic Standards](https://csrc.nist.gov/publications)
- [RFC 8439 - ChaCha20-Poly1305](https://tools.ietf.org/html/rfc8439)
- [RFC 9106 - Argon2](https://tools.ietf.org/html/rfc9106)
