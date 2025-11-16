# FileVault Architecture Guide

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     CLI Layer                            │
│  ┌────────────┐  ┌─────────────┐  ┌──────────────┐    │
│  │  Commands  │  │   Parser    │  │  Validator   │    │
│  │  (CLI11)   │  │             │  │              │    │
│  └─────┬──────┘  └──────┬──────┘  └──────┬───────┘    │
└────────┼─────────────────┼─────────────────┼───────────┘
         │                 │                 │
         └─────────────────┴─────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────┐
│                    Core Layer                            │
│  ┌──────────────────────────────────────────────────┐  │
│  │          CryptoEngine (Orchestrator)             │  │
│  └────┬────────────────┬────────────────┬───────────┘  │
│       │                │                │               │
│  ┌────▼─────┐    ┌────▼─────┐    ┌────▼──────┐       │
│  │ File     │    │ Key      │    │ Algorithm │       │
│  │ Handler  │    │ Manager  │    │ Registry  │       │
│  └──────────┘    └──────────┘    └───────────┘       │
└──────────────────────────────────────────────────────────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
┌────────▼────────┐ ┌──────▼──────┐ ┌───────▼────────┐
│   Algorithms    │ │     KDF     │ │  Compression   │
│                 │ │             │ │                │
│ • Classic       │ │ • Argon2    │ │ • zlib         │
│ • Symmetric     │ │ • PBKDF2    │ │ • bzip2        │
│ • Asymmetric    │ │ • scrypt    │ │ • LZMA         │
│ • Hash          │ │             │ │                │
└─────────────────┘ └─────────────┘ └────────────────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
┌────────▼────────┐ ┌──────▼──────┐ ┌───────▼────────┐
│     Botan 3.x   │ │   System    │ │   Utilities    │
│  Cryptography   │ │   Calls     │ │                │
│                 │ │             │ │ • Logger       │
│ • Ciphers       │ │ • Memory    │ │ • Formatter    │
│ • RNG           │ │   Lock      │ │ • Benchmarks   │
│ • KDF           │ │ • Keychain  │ │ • Progress     │
└─────────────────┘ └─────────────┘ └────────────────┘
```

---

## 🔄 Data Flow

### Encryption Flow

```
User Input
    │
    ├──> Password ──────────────────┐
    │                                │
    ├──> File Path                   │
    │      │                         │
    │      └──> FileHandler          │
    │             │                  │
    │             ├──> Read          │
    │             │    └──> Buffer   │
    │             │                  │
    │             └──> Validate      │
    │                  │             │
    ├──> Config        │             │
    │      │           │             │
    │      └──────┬────┘             │
    │             │                  │
    │             ▼                  ▼
    │        CryptoEngine      KeyManager
    │             │                  │
    │             ├──> Generate ─────┤
    │             │    Salt/Nonce    │
    │             │         │        │
    │             │         ├────────┤
    │             │         │  KDF   │
    │             │         │  (Argon2)
    │             │         │        │
    │             │         └────────┤
    │             │              Derived Key
    │             │                  │
    │             ├──> Algorithm <───┘
    │             │      (AES-GCM)
    │             │         │
    │             │         ├──> Encrypt
    │             │         │      │
    │             │         │      └──> Ciphertext + Tag
    │             │         │              │
    │             ├─────────┴──────────────┤
    │             │   Build File Format    │
    │             │   ┌──────────────────┐ │
    │             │   │ Magic Header     │ │
    │             │   │ Salt (32 bytes)  │ │
    │             │   │ Nonce (12 bytes) │ │
    │             │   │ Tag (16 bytes)   │ │
    │             │   │ Ciphertext       │ │
    │             │   └──────────────────┘ │
    │             │                        │
    │             ▼                        │
    │        FileHandler                   │
    │             │                        │
    │             └──> Write ──────────────┘
    │                    │
    └────────────────────┼──> Success
                         │
                    Output File
```

### Decryption Flow

```
Encrypted File
    │
    └──> FileHandler
           │
           ├──> Read
           │    └──> Parse Header
           │           │
           │           ├──> Extract Salt
           │           ├──> Extract Nonce
           │           ├──> Extract Tag
           │           └──> Extract Ciphertext
           │
User Password
    │
    ├──> KeyManager
    │       │
    │       └──> KDF (with extracted Salt)
    │              │
    │              └──> Derived Key
    │                      │
    └──────────────────────┤
                           │
                    CryptoEngine
                           │
                           ├──> Get Algorithm
                           │      │
                           │      └──> AES-GCM Decrypt
                           │             │
                           │             ├──> Verify Tag
                           │             │      │
                           │             │      ├──> Success ✓
                           │             │      └──> Fail ✗
                           │             │
                           │             └──> Plaintext
                           │
                           └──> FileHandler
                                  │
                                  └──> Write
                                         │
                                  Decrypted File
```

---

## 🧩 Component Details

### 1. CryptoEngine (Core Orchestrator)

**Responsibilities:**
- Algorithm registry and selection
- Coordination between components
- Error handling and validation
- Performance monitoring

**Key Methods:**
```cpp
class CryptoEngine {
    // Algorithm management
    void register_algorithm(unique_ptr<ICryptoAlgorithm> algo);
    ICryptoAlgorithm* get_algorithm(AlgorithmType type);
    
    // Key operations
    SecureVector<uint8_t> derive_key(const SecureString& password,
                                       span<const uint8_t> salt,
                                       const EncryptionConfig& config);
    
    // High-level operations
    Result<EncryptedData> encrypt_file(const string& path, 
                                        const SecureString& password,
                                        const EncryptionConfig& config);
    
    Result<vector<uint8_t>> decrypt_file(const string& path,
                                          const SecureString& password);
};
```

### 2. ICryptoAlgorithm (Algorithm Interface)

**Contract:**
```cpp
class ICryptoAlgorithm {
public:
    virtual ~ICryptoAlgorithm() = default;
    
    // Identity
    virtual string name() const = 0;
    virtual AlgorithmType type() const = 0;
    virtual size_t key_size() const = 0;
    
    // Operations
    virtual CryptoResult encrypt(span<const uint8_t> plaintext,
                                 span<const uint8_t> key,
                                 const EncryptionConfig& config) = 0;
    
    virtual CryptoResult decrypt(span<const uint8_t> ciphertext,
                                 span<const uint8_t> key,
                                 const EncryptionConfig& config) = 0;
    
    // Metadata
    virtual bool is_suitable_for(UserLevel level) const = 0;
};
```

**Implementation Example:**
```cpp
class AES256GCM : public ICryptoAlgorithm {
    // Botan 3.x integration
    unique_ptr<Botan::Cipher_Mode> cipher_;
    
    CryptoResult encrypt(...) override {
        // 1. Generate unique nonce
        // 2. Setup cipher with key
        // 3. Process data
        // 4. Return ciphertext + tag
    }
};
```

### 3. FileHandler (I/O Management)

**Responsibilities:**
- Safe file reading/writing
- Format parsing/serialization
- Memory-mapped I/O for large files
- Progress reporting

```cpp
class FileHandler {
    // Reading
    vector<uint8_t> read_file(const string& path);
    EncryptedFileHeader parse_header(span<const uint8_t> data);
    
    // Writing
    void write_file(const string& path, span<const uint8_t> data);
    void write_with_header(const string& path, 
                          const EncryptedFileHeader& header,
                          span<const uint8_t> ciphertext);
    
    // Streaming (for large files)
    class FileStream {
        void read_chunk(size_t offset, size_t length);
        void write_chunk(span<const uint8_t> data);
    };
};
```

### 4. File Format Specification

```
FileVault Encrypted File (.fv)
┌──────────────────────────────────────┐
│ Magic (8 bytes):     "FVAULT01"      │  Fixed identifier
├──────────────────────────────────────┤
│ Version (4 bytes):   0x00000001      │  Format version
├──────────────────────────────────────┤
│ Flags (4 bytes):                     │  Feature flags
│   Bit 0: Compressed                  │
│   Bit 1: Quantum-resistant           │
│   Bit 2-31: Reserved                 │
├──────────────────────────────────────┤
│ Algorithm ID (4 bytes):              │  e.g., 0x0001 = AES-256-GCM
├──────────────────────────────────────┤
│ KDF Type (4 bytes):                  │  e.g., 0x0001 = Argon2id
├──────────────────────────────────────┤
│ KDF Iterations (4 bytes):            │  Number of iterations
├──────────────────────────────────────┤
│ KDF Memory (4 bytes):                │  Memory usage (KB)
├──────────────────────────────────────┤
│ KDF Parallelism (4 bytes):           │  Thread count
├──────────────────────────────────────┤
│ Salt Length (4 bytes):               │  Typically 32
├──────────────────────────────────────┤
│ Salt (variable):                     │  UNIQUE random bytes
├──────────────────────────────────────┤
│ Nonce Length (4 bytes):              │  Typically 12 for GCM
├──────────────────────────────────────┤
│ Nonce (variable):                    │  UNIQUE random bytes
├──────────────────────────────────────┤
│ Original Size (8 bytes):             │  Before compression/encryption
├──────────────────────────────────────┤
│ Compressed Size (8 bytes):           │  After compression (if any)
├──────────────────────────────────────┤
│ Tag Length (4 bytes):                │  Authentication tag (16 for GCM)
├──────────────────────────────────────┤
│ Tag (variable):                      │  GCM authentication tag
├──────────────────────────────────────┤
│ Metadata Length (4 bytes):           │  JSON metadata
├──────────────────────────────────────┤
│ Metadata (variable):                 │  Optional JSON
│   {                                  │
│     "timestamp": "...",              │
│     "comment": "...",                │
│     "original_name": "..."           │
│   }                                  │
├──────────────────────────────────────┤
│ Ciphertext (variable):               │  Encrypted data
│ ...                                  │
└──────────────────────────────────────┘

Total Header Size: ~128-256 bytes (variable)
```

---

## 🔐 Security Architecture

### Defense in Depth Layers

```
Layer 1: Password Protection
    └─> Strong password policy
    └─> Password strength meter
    └─> Option: Keyfile support

Layer 2: Key Derivation (Argon2id)
    └─> Memory-hard (anti-GPU)
    └─> Unique salt per file
    └─> High iteration count
    └─> Optional pepper

Layer 3: Encryption (AES-256-GCM)
    └─> 256-bit key
    └─> Authenticated encryption (AEAD)
    └─> Unique nonce per encryption
    └─> 128-bit authentication tag

Layer 4: Memory Protection
    └─> Secure allocation
    └─> Memory locking
    └─> Zeroing on free
    └─> RAII for cleanup

Layer 5: Implementation Security
    └─> Constant-time operations
    └─> Timing attack protection
    └─> No side-channel leaks
    └─> Validated with NIST vectors
```

### Threat Model

**Protected Against:**
- ✅ Brute force attacks (strong KDF)
- ✅ Rainbow tables (unique salt)
- ✅ Dictionary attacks (password strength)
- ✅ Known-plaintext (unique nonce)
- ✅ Chosen-ciphertext (authenticated encryption)
- ✅ Timing attacks (constant-time ops)
- ✅ Memory dumps (secure memory)

**NOT Protected Against:**
- ❌ Rubber-hose cryptanalysis (physical coercion)
- ❌ Keyloggers on compromised system
- ❌ Evil maid attacks (physical access)
- ❌ Quantum computers (in classical mode)

---

## 📊 Performance Considerations

### Optimization Strategy

```
Priority 1: Security (never compromise)
    └─> Always use secure algorithms
    └─> Never skip validation
    └─> Always clear sensitive data

Priority 2: Correctness (validate everything)
    └─> NIST test vectors
    └─> Fuzz testing
    └─> Edge case handling

Priority 3: Performance (optimize after)
    └─> Benchmark before optimizing
    └─> Profile to find hotspots
    └─> Use SIMD when available
```

### Benchmarking Points

```cpp
// Key areas to benchmark
1. Key Derivation (KDF)
   - Argon2: ~100-500ms (intentionally slow)
   - PBKDF2: ~50-200ms
   - scrypt: ~100-400ms

2. Encryption/Decryption
   - AES-GCM: ~500 MB/s (software)
   - ChaCha20: ~800 MB/s (software)
   - AES-NI: ~2-4 GB/s (hardware)

3. Compression
   - zlib: ~100 MB/s
   - bzip2: ~10 MB/s (better ratio)
   - LZMA: ~5 MB/s (best ratio)

4. File I/O
   - Sequential read: ~1 GB/s (SSD)
   - Random read: ~100 MB/s (SSD)
```

---

## 🧪 Testing Strategy

### Test Pyramid

```
                    /\
                   /  \
                  / E2E\    (Few)
                 /------\
                /  Integ \  (Some)
               /----------\
              /   Unit     \ (Many)
             /--------------\
            /  NIST Vectors  \ (Extensive)
           /------------------\
```

**Test Coverage Requirements:**
- Unit tests: 80%+ coverage
- NIST vectors: 100% pass
- Integration: All workflows
- Fuzzing: Continuous

---

## 🔄 Extension Points

### Adding New Algorithms

1. Implement `ICryptoAlgorithm` interface
2. Add to algorithm registry
3. Create unit tests
4. Add NIST test vectors (if available)
5. Update documentation
6. Benchmark performance

### Adding New Features

1. Design interface first
2. Write tests (TDD)
3. Implement feature
4. Document thoroughly
5. Update architecture docs

---

## 📚 References

- **Botan 3.x Documentation**: https://botan.randombit.net/handbook/
- **NIST Standards**: https://csrc.nist.gov/
- **OWASP Cheat Sheets**: https://cheatsheetseries.owasp.org/
- **Cryptography Guidelines**: https://www.keylength.com/

---

## 🎯 Design Principles

1. **Security by Default**: Safe choices out of the box
2. **Defense in Depth**: Multiple security layers
3. **Fail Securely**: Errors should be safe
4. **Clear over Clever**: Readable > Concise
5. **Test Everything**: Verify, don't assume
6. **Document Always**: Code explains how, docs explain why