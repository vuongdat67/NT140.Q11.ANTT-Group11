# FileVault Algorithms Documentation

Comprehensive guide to cryptographic algorithms supported by FileVault.

## Table of Contents
- [Symmetric Encryption](#symmetric-encryption)
  - [AEAD Ciphers](#aead-ciphers)
  - [Block Cipher Modes](#block-cipher-modes)
- [Asymmetric Encryption](#asymmetric-encryption)
- [Post-Quantum Cryptography](#post-quantum-cryptography)
- [Key Derivation Functions](#key-derivation-functions)
- [Hash Functions](#hash-functions)
- [Classical Ciphers](#classical-ciphers)

---

## Symmetric Encryption

### AEAD Ciphers
**AEAD** (Authenticated Encryption with Associated Data) provides both **confidentiality** and **authenticity** in a single operation.

#### AES-GCM (Galois/Counter Mode)

**Standards:** NIST SP 800-38D, FIPS 197  
**Key Sizes:** 128, 192, 256 bits  
**Performance:** ⚡⚡⚡⚡ (Hardware accelerated with AES-NI)

**Description:**  
AES-GCM combines CTR mode encryption with GMAC authentication. It's the most widely used AEAD cipher and provides excellent performance on modern CPUs with hardware acceleration.

**Security Features:**
- 128-bit authentication tag
- Nonce-based (96-bit IV standard)
- Parallelizable encryption/decryption
- Resistant to chosen-ciphertext attacks

**Usage:**
```bash
filevault encrypt document.pdf -a aes-256-gcm
```

**When to Use:**
- ✅ General purpose encryption (default choice)
- ✅ High-performance requirements
- ✅ Modern hardware with AES-NI
- ✅ Compliance requirements (NIST approved)

**Security Considerations:**
- ⚠️ Nonce reuse is catastrophic (breaks authentication)
- ⚠️ Maximum 2^32 blocks per key (64 GB with 128-bit blocks)
- ✅ FileVault automatically manages nonces and enforces limits

---

#### ChaCha20-Poly1305

**Standards:** RFC 8439  
**Key Size:** 256 bits  
**Performance:** ⚡⚡⚡⚡ (Software optimized)

**Description:**  
Modern stream cipher (ChaCha20) combined with Poly1305 MAC. Designed by Daniel J. Bernstein as an alternative to AES-GCM for systems without hardware AES acceleration.

**Security Features:**
- 256-bit key (no 128-bit variant)
- 96-bit nonce + 32-bit counter
- Constant-time implementation (resistant to timing attacks)
- No cache-timing vulnerabilities

**Usage:**
```bash
filevault encrypt data.zip -a chacha20-poly1305
```

**When to Use:**
- ✅ Mobile devices / ARM processors
- ✅ Systems without AES-NI
- ✅ When constant-time guarantees are critical
- ✅ IoT devices

**Security Considerations:**
- ✅ Simpler design than AES-GCM
- ✅ Better software performance on ARM
- ✅ Widely deployed (TLS 1.3, WireGuard, Signal)

---

#### Serpent-256-GCM

**Standards:** AES finalist  
**Key Size:** 256 bits  
**Performance:** ⚡⚡ (Conservative design)

**Description:**  
Serpent was an AES finalist known for its conservative security margin. Combined with GCM for authentication.

**Security Features:**
- 32 rounds (vs AES's 14 rounds for 256-bit)
- Designed for maximum security over performance
- No known practical attacks

**Usage:**
```bash
filevault encrypt sensitive.doc -a serpent-256-gcm
```

**When to Use:**
- ✅ Maximum security requirements
- ✅ Long-term data protection
- ❌ Performance is not critical

---

#### International Standard Ciphers

##### Camellia-GCM (Japan)
**Standards:** ISO/IEC 18033-3, CRYPTREC, NESSIE  
**Key Sizes:** 128, 256 bits

Developed by Mitsubishi and NTT. Similar design to AES but with different S-boxes.

```bash
filevault encrypt file.txt -a camellia-256-gcm
```

##### ARIA-GCM (Korea)
**Standards:** KS X 1213, RFC 5794, ISO/IEC 18033-3  
**Key Sizes:** 128, 256 bits

Korean national standard. Block cipher similar to AES structure.

```bash
filevault encrypt file.txt -a aria-256-gcm
```

##### SM4-GCM (China)
**Standards:** GB/T 32907-2016  
**Key Size:** 128 bits only

Chinese national standard cipher. Used in Chinese financial and government systems.

```bash
filevault encrypt file.txt -a sm4-gcm
```

---

### Block Cipher Modes

⚠️ **Warning:** These modes do NOT provide authentication. Use AEAD ciphers unless you have specific requirements.

#### CBC (Cipher Block Chaining)

**Description:** Each plaintext block is XORed with the previous ciphertext block before encryption.

**Security:**
- ❌ No authentication (vulnerable to tampering)
- ❌ Padding oracle attacks if not properly implemented
- ❌ Not parallelizable for encryption

```bash
filevault encrypt file.txt -a aes-256-cbc
```

#### CTR (Counter Mode)

**Description:** Turns block cipher into stream cipher by encrypting counter values.

**Security:**
- ✅ Parallelizable
- ❌ No authentication
- ⚠️ Nonce reuse breaks confidentiality

```bash
filevault encrypt file.txt -a aes-256-ctr
```

#### CFB/OFB (Cipher Feedback / Output Feedback)

**Description:** Stream cipher modes with self-synchronization (CFB) or pre-computed keystream (OFB).

```bash
filevault encrypt file.txt -a aes-256-cfb
filevault encrypt file.txt -a aes-256-ofb
```

#### XTS (XEX-based Tweaked Codebook)

**Standards:** IEEE P1619  
**Use Case:** Disk encryption

**Description:** Designed for encrypting data at rest (disk sectors). Uses two keys: one for encryption, one for tweaking.

**Security:**
- ✅ Sector-based tweaking prevents block shuffling
- ❌ No authentication (disk encryption typically doesn't need it)
- ✅ Efficient random access

```bash
filevault encrypt disk.img -a aes-256-xts
```

---

## Asymmetric Encryption

### RSA (Rivest-Shamir-Adleman)

**Standards:** PKCS#1, FIPS 186-4  
**Key Sizes:** 2048, 3072, 4096 bits

**Description:**  
Classic public-key cryptosystem based on integer factorization problem.

**Security Levels:**
- RSA-2048: 112-bit security (minimum acceptable)
- RSA-3072: 128-bit security (recommended)
- RSA-4096: 140-bit security (high security)

**Performance:**
- Key Generation: Slow (90ms - 1.7s)
- Encryption: Fast (~0.5ms)
- Decryption: Slow (3-18ms)

**Usage:**
```bash
# Generate keypair
filevault keygen -a rsa-4096 -o mykey

# Encrypt
filevault encrypt file.txt --pubkey mykey.pub

# Decrypt
filevault decrypt file.txt.fvlt --privkey mykey.pem
```

**When to Use:**
- ✅ Key exchange
- ✅ Digital signatures
- ✅ Legacy system compatibility
- ❌ Bulk data encryption (slow)

**Limitations:**
- Maximum message size = key size - padding overhead
- Vulnerable to quantum computers (Shor's algorithm)
- Large key sizes (4096 bits = 512 bytes)

---

### ECC (Elliptic Curve Cryptography)

**Standards:** NIST P-curves (FIPS 186-4)  
**Curves:** P-256, P-384, P-521

**Description:**  
Public-key cryptography based on elliptic curve discrete logarithm problem. Provides same security as RSA with much smaller keys.

**Security Levels:**
- ECC-P256: 128-bit security ≈ RSA-3072
- ECC-P384: 192-bit security ≈ RSA-7680
- ECC-P521: 256-bit security ≈ RSA-15360

**FileVault Implementation:**
Uses ECDH (Elliptic Curve Diffie-Hellman) to establish shared secret, then encrypts with AES-GCM.

**Performance:**
- Key Generation: Fast (~1-10ms)
- Encryption: Fast (~0.5-1.7ms)
- Decryption: Fast (~0.3-1.3ms)

**Usage:**
```bash
# Generate keypair
filevault keygen -a ecc-p521 -o mykey

# Encrypt (hybrid: ECDH + AES-GCM)
filevault encrypt largefile.zip --pubkey mykey.pub
```

**When to Use:**
- ✅ Modern applications
- ✅ Smaller key sizes needed
- ✅ Bulk data encryption (hybrid mode)
- ✅ Mobile/embedded systems

**Advantages over RSA:**
- Smaller keys (256-bit ECC ≈ 3072-bit RSA)
- Faster operations
- Lower bandwidth requirements

---

## Post-Quantum Cryptography

**Background:**  
Quantum computers running Shor's algorithm can break RSA and ECC. NIST selected quantum-resistant algorithms in 2022.

### Kyber (ML-KEM - Module Learning with Errors)

**Standard:** NIST FIPS 203 (draft)  
**Type:** Key Encapsulation Mechanism (KEM)  
**Variants:** Kyber-512, Kyber-768, Kyber-1024

**Description:**  
Lattice-based cryptography resistant to quantum attacks. Winner of NIST PQC standardization.

**Security Levels:**
- Kyber-512: NIST Level 1 (128-bit quantum security)
- Kyber-768: NIST Level 3 (192-bit quantum security)
- Kyber-1024: NIST Level 5 (256-bit quantum security) ⭐ Recommended

**Performance:**
- Key Generation: ⚡⚡⚡⚡ (0.2-0.4 ms)
- Encapsulation: ⚡⚡⚡⚡ (0.3-0.6 ms)
- Decapsulation: ⚡⚡⚡⚡ (0.4-0.8 ms)

**Key Sizes:**
- Kyber-1024: Public key ~1568 bytes, Private key ~3168 bytes
- Much larger than ECC but acceptable

**Usage:**
```bash
# Generate Kyber keypair
filevault keygen -a kyber-1024 -o quantum-key

# Note: Kyber is a KEM, not direct encryption
# Use Kyber-Hybrid for file encryption
```

---

### KyberHybrid (PQC + Classical)

**Description:**  
Combines Kyber KEM with AES-256-GCM for defense-in-depth strategy. Protects against:
- Quantum attacks (Kyber)
- Classical attacks (AES-GCM)
- Unknown vulnerabilities in either

**Architecture:**
1. Generate Kyber-1024 shared secret
2. Derive AES-256-GCM key from shared secret
3. Encrypt data with AES-GCM

**Performance:**
- Overhead: Minimal (~0.5ms for key exchange)
- Throughput: ~650 MB/s (same as AES-GCM)

**Usage:**
```bash
# Hybrid encryption (recommended for quantum threat)
filevault encrypt secret.zip -a kyber-1024-hybrid

# Requires Kyber keypair
filevault keygen -a kyber-1024 -o key
filevault encrypt data.txt -a kyber-1024-hybrid --pubkey key.pub
```

**When to Use:**
- ✅ Long-term data protection (10+ years)
- ✅ "Store now, decrypt later" threat model
- ✅ Compliance with quantum-readiness
- ✅ Defense-in-depth strategy

---

### Dilithium (ML-DSA - Module Lattice Digital Signature)

**Standard:** NIST FIPS 204 (draft)  
**Type:** Digital Signature  
**Variants:** Dilithium-2, Dilithium-3, Dilithium-5

**Description:**  
Lattice-based digital signature scheme. Provides quantum-resistant authentication.

**Security Levels:**
- Dilithium-2: NIST Level 2
- Dilithium-3: NIST Level 3
- Dilithium-5: NIST Level 5 ⭐ Maximum security

**Performance:**
- Key Generation: ⚡⚡⚡⚡ (0.2-0.5 ms)
- Sign: ⚡⚡⚡ (0.4-1.6 ms)
- Verify: ⚡⚡⚡⚡ (0.3-0.8 ms)

**Signature Sizes:**
- Dilithium-5: ~4595 bytes (large but acceptable)

**Usage:**
```bash
# Generate Dilithium keypair
filevault keygen -a dilithium-5 -o signature-key

# Future: Digital signatures for file authenticity
# filevault sign file.txt --privkey signature-key.pem
# filevault verify file.txt.sig --pubkey signature-key.pub
```

---

## Key Derivation Functions

### Argon2id (Recommended)

**Standards:** RFC 9106, PHC winner  
**Type:** Memory-hard KDF

**Description:**  
Hybrid of Argon2i (data-independent) and Argon2d (data-dependent). Best resistance against GPU/ASIC attacks and side-channel attacks.

**Parameters:**
- Memory: 64 MB - 512 MB
- Iterations: 3-50
- Parallelism: CPU cores

**Security:**
- ✅ Memory-hard (expensive for attackers)
- ✅ Resistant to timing attacks
- ✅ Configurable security levels

**Usage:**
```bash
filevault encrypt file.txt -k argon2id -s strong
```

**Security Levels:**
| Level    | Iterations | Memory | Time   |
|----------|------------|--------|--------|
| weak     | 1          | 4MB    | ~2ms   |
| medium   | 2          | 16MB   | ~10ms  |
| strong   | 3          | 64MB   | ~30ms  |
| paranoid | 4          | 128MB  | ~60ms  |

---

### PBKDF2-SHA256/SHA512

**Standards:** RFC 8018, NIST SP 800-132  
**Type:** Password-based KDF

**Description:**  
Classic KDF using iterated hash functions. Less secure than Argon2 but widely supported.

**Security:**
- ❌ Not memory-hard (vulnerable to GPU attacks)
- ✅ Well-studied and standardized
- ✅ Fast to compute

**When to Use:**
- Legacy compatibility
- Constrained environments
- Compliance requirements

---

## Hash Functions

### SHA-256 (Recommended)

**Standards:** FIPS 180-4  
**Output:** 256 bits  
**Performance:** ~1800 MB/s

Widely used, well-studied, suitable for most applications.

### SHA-512

**Standards:** FIPS 180-4  
**Output:** 512 bits  
**Performance:** ~400 MB/s

Stronger but slower. Use for long-term security.

### SHA3-256/SHA3-512

**Standards:** FIPS 202  
**Type:** Keccak sponge construction

NIST's latest standard. Different design from SHA-2, provides diversity.

### BLAKE2b

**Output:** 512 bits  
**Performance:** ~810 MB/s ⚡⚡⚡

Modern, fast, secure. Preferred for new applications.

---

## Classical Ciphers

⚠️ **FOR EDUCATIONAL PURPOSES ONLY - COMPLETELY INSECURE**

### Caesar Cipher
Simple shift cipher. Breakable by brute-force (26 keys).

### Vigenère Cipher
Polyalphabetic substitution. Breakable by Kasiski examination.

### Playfair Cipher
Digraph substitution. Breakable by frequency analysis.

### Hill Cipher
Matrix-based encryption. Vulnerable to known-plaintext attacks.

### Substitution Cipher
Monoalphabetic substitution. Breakable by frequency analysis.

**Usage (educational only):**
```bash
filevault encrypt msg.txt -a caesar -p "key"
filevault encrypt msg.txt -a vigenere -p "secretkey"
```

---

## Algorithm Selection Guide

### General Purpose
→ **AES-256-GCM** (default, best balance)

### Maximum Security
→ **Kyber-1024-Hybrid** (quantum-resistant)

### High Performance
→ **ChaCha20-Poly1305** (ARM/mobile)  
→ **AES-256-GCM** (x86 with AES-NI)

### Long-term Storage
→ **Kyber-1024-Hybrid** (quantum threat)  
→ **Serpent-256-GCM** (conservative classical)

### Legacy Compatibility
→ **RSA-3072** (asymmetric)  
→ **AES-256-CBC** (symmetric)

### Compliance
- **FIPS 140-2:** AES-GCM, RSA, ECC, SHA-2
- **NIST PQC:** Kyber (ML-KEM), Dilithium (ML-DSA)
- **International:** Camellia, ARIA, SM4

---

## Security Best Practices

1. **Always use AEAD ciphers** (GCM mode) for new applications
2. **Use Argon2id** for password-based encryption
3. **Consider PQC** for data stored >5 years
4. **Never reuse nonces/IVs** (FileVault handles this automatically)
5. **Use 256-bit keys** when possible
6. **Avoid ECB mode** (NEVER use in production)
7. **Keep software updated** (crypto vulnerabilities are discovered)

---

## References

- [NIST Post-Quantum Cryptography](https://csrc.nist.gov/projects/post-quantum-cryptography)
- [RFC 8439 - ChaCha20-Poly1305](https://tools.ietf.org/html/rfc8439)
- [NIST SP 800-38D - AES-GCM](https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38d.pdf)
- [Botan Crypto Library](https://botan.randombit.net/)
