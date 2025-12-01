# 🛡️ Kyber (ML-KEM) - Post-Quantum Key Encapsulation

> **Thuật toán mã hóa chống lại máy tính lượng tử - NIST PQC Selected Standard.**

---

## 📋 Mục lục

1. [Tóm tắt](#-tóm-tắt)
2. [Tại sao cần Post-Quantum?](#-tại-sao-cần-post-quantum)
3. [Lattice-based Cryptography](#-lattice-based-cryptography)
4. [Cách hoạt động Kyber](#-cách-hoạt-động-kyber)
5. [Hybrid Mode trong FileVault](#-hybrid-mode-trong-filevault)
6. [Implement trong FileVault](#-implement-trong-filevault)
7. [Security Levels](#-security-levels)
8. [Test Vectors](#-test-vectors)
9. [Tham khảo](#-tham-khảo)

---

## 📝 Tóm tắt

| Thuộc tính | Kyber-512 | Kyber-768 | Kyber-1024 |
|------------|-----------|-----------|------------|
| **NIST Level** | Level 1 | Level 3 | Level 5 |
| **Security** | 128-bit | 192-bit | 256-bit |
| **Public key** | 800 bytes | 1,184 bytes | 1,568 bytes |
| **Private key** | 1,632 bytes | 2,400 bytes | 3,168 bytes |
| **Ciphertext** | 768 bytes | 1,088 bytes | 1,568 bytes |
| **Shared secret** | 32 bytes | 32 bytes | 32 bytes |

### Kyber là gì?

```
┌─────────────────────────────────────────────────────────────────────┐
│                        KYBER Overview                               │
└─────────────────────────────────────────────────────────────────────┘

Kyber là KEM (Key Encapsulation Mechanism):
├── KHÔNG mã hóa trực tiếp dữ liệu
├── Tạo ra shared secret an toàn giữa 2 bên
├── Shared secret dùng để derive AES/ChaCha key
└── Dựa trên Module-LWE (Learning With Errors)

    Alice                                          Bob
    ─────                                          ───
    (pk, sk) ← KeyGen()
          pk ─────────────────────────────────────▶
                                    (ct, ss) ← Encaps(pk)
              ◀─────────────────────────────────── ct
    ss ← Decaps(sk, ct)
    
    Cả hai có shared secret (ss) = 32 bytes
    → Dùng ss để derive AES-256-GCM key
```

---

## 🔮 Tại sao cần Post-Quantum?

### Quantum Threat

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Quantum Computer Threat                          │
└─────────────────────────────────────────────────────────────────────┘

Thuật toán Shor (Peter Shor, 1994):
├── Phá RSA, ECC, DH trong polynomial time
├── RSA-2048: Cần ~4000 logical qubits
├── ECC-256: Cần ~2000 logical qubits
└── Timeline: 10-20 năm (có thể sớm hơn)

                    Classical        Quantum (Shor)     Quantum-Safe?
                    ─────────        ──────────────     ─────────────
RSA-2048            2^112            Polynomial         ❌ BROKEN
ECC P-256           2^128            Polynomial         ❌ BROKEN
AES-256             2^256            2^128 (Grover)     ✅ SAFE (still strong)
Kyber-768           2^192            2^192              ✅ SAFE

⚠️ "Harvest Now, Decrypt Later" Attack:
   - Adversary thu thập encrypted data NGAY BÂY GIỜ
   - Chờ quantum computer ready
   - Giải mã TẤT CẢ dữ liệu đã thu thập
```

### NIST PQC Competition

```
Timeline:
├── 2016: NIST bắt đầu competition
├── 2017: 69 submissions
├── 2020: Round 3 finalists
├── 2022: CRYSTALS-Kyber selected! ✓
├── 2024: FIPS 203 (ML-KEM) published
└── Now: Kyber = NIST standard

Tại sao Kyber thắng?
├── Performance tốt nhất trong finalists
├── Key/ciphertext size hợp lý
├── Được nghiên cứu kỹ lưỡng
├── Dựa trên hard problem (Module-LWE)
└── Constant-time implementation khả thi
```

---

## 📐 Lattice-based Cryptography

### Lattice là gì?

```
┌─────────────────────────────────────────────────────────────────────┐
│                    2D Lattice Example                               │
└─────────────────────────────────────────────────────────────────────┘

    Lattice = tập hợp các điểm tạo bởi linear combination của basis vectors
    
    Basis: b₁ = (3, 1), b₂ = (1, 2)
    
         •       •       •       •       •
              
         •       •       •       •       •
                    
         •       •   ●   •       •       •   ← Điểm lattice
                    ↑
         •       • Origin  •       •       •
                    
         •       •       •       •       •

    Lattice point = n₁·b₁ + n₂·b₂  với n₁, n₂ ∈ ℤ
```

### Learning With Errors (LWE)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LWE Problem                                      │
└─────────────────────────────────────────────────────────────────────┘

Setup:
├── Secret vector: s ∈ ℤₙ
├── Public matrix: A ∈ ℤₙₓₘ (random)
├── Error vector: e ∈ ℤₘ (small, Gaussian)
└── Compute: b = A·s + e (mod q)

Problem: Cho (A, b), tìm s

    ┌───────────────────────────────────────────────────────────┐
    │   A (public)      ×    s (secret)   +   e (error)  =  b   │
    │                                                           │
    │   ┌───┐           ┌───┐             ┌───┐          ┌───┐ │
    │   │ ▓ │     ×     │ ▓ │      +      │ ░ │    =     │ ▓ │ │
    │   │ ▓ │           │ ▓ │             │ ░ │          │ ▓ │ │
    │   │ ▓ │           │ ▓ │             │ ░ │          │ ▓ │ │
    │   │ ▓ │           └───┘             │ ░ │          │ ▓ │ │
    │   └───┘                             └───┘          └───┘ │
    │                                                           │
    │   n×m              n×1    (small)    m×1           m×1   │
    └───────────────────────────────────────────────────────────┘

    Không có error (e=0): Dễ giải bằng Gaussian elimination
    Có error nhỏ: Extremely hard! (NP-hard related)
```

### Module-LWE (Kyber sử dụng)

```
Module-LWE = LWE trên ring Rq = ℤq[X]/(X^n + 1)

Advantages:
├── Smaller key sizes (structured)
├── Faster operations (NTT - Number Theoretic Transform)
├── Same hardness assumptions
└── n = 256, q = 3329 trong Kyber

    Matrix elements are polynomials:
    
    a ∈ Rq = a₀ + a₁X + a₂X² + ... + a₂₅₅X²⁵⁵
    
    Multiplication uses NTT (similar to FFT):
    ├── a·b trong time domain: O(n²)
    └── NTT(a)·NTT(b): O(n log n)
```

---

## ⚙️ Cách hoạt động Kyber

### Key Generation

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Kyber.KeyGen()                                   │
└─────────────────────────────────────────────────────────────────────┘

    1. Sample matrix A ∈ Rq^(k×k) từ random seed ρ
    2. Sample secret s ∈ Rq^k với small coefficients
    3. Sample error e ∈ Rq^k với small coefficients  
    4. Compute t = A·s + e
    
    Public Key:  pk = (ρ, t)        ← Encode and output
    Private Key: sk = s              ← Keep secret!
    
    Visualization:
    
        ┌─────────────────────────────────────────┐
        │           Key Generation                │
        └─────────────────────────────────────────┘
        
        Random seed ρ
              │
              ▼
        ┌───────────┐
        │  Expand   │ ──────▶ Matrix A (k×k polynomials)
        │  (SHAKE)  │
        └───────────┘
              │
              │     s (secret)    e (error)
              │         │             │
              │         ▼             ▼
              └────▶  A × s    +      e     =  t
                            
        Public key = (ρ, t)
        Private key = s
```

### Encapsulation (Encrypt shared secret)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Kyber.Encaps(pk)                                 │
└─────────────────────────────────────────────────────────────────────┘

    Input: pk = (ρ, t)
    
    1. Sample random message m ∈ {0,1}^256
    2. Derive (r, e₁, e₂) from m using hash
    3. Reconstruct A from ρ
    4. Compute:
       - u = A^T · r + e₁           (vector of polynomials)
       - v = t^T · r + e₂ + ⌈q/2⌋·m  (single polynomial + encoded message)
    
    Ciphertext: ct = (u, v)   ← Compress and output
    Shared secret: ss = H(m)  ← Hash of original message
    
        ┌─────────────────────────────────────────┐
        │           Encapsulation                 │
        └─────────────────────────────────────────┘
        
        Random m (256 bits)
              │
              ├─────────▶ r, e₁, e₂ (via hash)
              │
              ▼
        ┌───────────┐
        │  Compute  │
        │  u = Aᵀr+e₁
        │  v = tᵀr+e₂+encode(m)
        └─────┬─────┘
              │
              ├────────▶ Ciphertext ct = (u, v)
              │
              ▼
        ss = SHAKE256(m)  ← Shared secret (32 bytes)
```

### Decapsulation (Decrypt shared secret)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Kyber.Decaps(sk, ct)                             │
└─────────────────────────────────────────────────────────────────────┘

    Input: sk = s, ct = (u, v)
    
    1. Compute: v - s^T · u
       = (t^T·r + e₂ + ⌈q/2⌋·m) - s^T·(A^T·r + e₁)
       = (A·s + e)^T·r + e₂ + ⌈q/2⌋·m - s^T·A^T·r - s^T·e₁
       = s^T·A^T·r + e^T·r + e₂ + ⌈q/2⌋·m - s^T·A^T·r - s^T·e₁
       = e^T·r + e₂ - s^T·e₁ + ⌈q/2⌋·m
       ≈ ⌈q/2⌋·m                       (errors cancel out!)
    
    2. Decode m from noisy signal
    3. Re-encapsulate to verify (FO transform)
    4. If valid: ss = H(m)
    
        ┌─────────────────────────────────────────┐
        │           Decapsulation                 │
        └─────────────────────────────────────────┘
        
        Ciphertext (u, v)     Secret key s
              │                     │
              │                     │
              ▼                     ▼
        ┌─────────────────────────────────┐
        │   v - sᵀu ≈ ⌈q/2⌋·m + noise    │
        └─────────────────────────────────┘
              │
              ▼
        ┌───────────┐
        │  Decode   │ ──────▶ m (256 bits)
        │  (round)  │
        └───────────┘
              │
              ▼
        ss = SHAKE256(m)  ← Same shared secret!
```

---

## 🔗 Hybrid Mode trong FileVault

### Tại sao Hybrid?

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Hybrid Encryption Strategy                       │
└─────────────────────────────────────────────────────────────────────┘

Pure Kyber: Chỉ tin tưởng vào lattice hardness
Pure AES:   Tin tưởng vào classical security

Hybrid = Kyber + Classical → "Defense in Depth"

├── Nếu Kyber bị phá: Classical vẫn bảo vệ
├── Nếu AES bị phá: Kyber vẫn bảo vệ
└── Cần phá CẢ HAI để decrypt

    Security = max(Kyber security, Classical security)
```

### Kyber-Hybrid Flow trong FileVault

```
┌─────────────────────────────────────────────────────────────────────┐
│           FileVault Kyber-1024-Hybrid Encryption                    │
└─────────────────────────────────────────────────────────────────────┘

    ENCRYPTION:
    ───────────
    
    ┌──────────────┐     ┌──────────────┐
    │ Kyber-1024   │     │   Password   │
    │  Key Pair    │     │  (user)      │
    └──────┬───────┘     └──────┬───────┘
           │                    │
           ▼                    ▼
    ┌──────────────┐     ┌──────────────┐
    │   Encaps     │     │   Argon2id   │
    │   (pk)       │     │   KDF        │
    └──────┬───────┘     └──────┬───────┘
           │                    │
           ▼                    ▼
    ┌──────────────┐     ┌──────────────┐
    │ Kyber shared │     │ Password-    │
    │ secret (32B) │     │ derived key  │
    └──────┬───────┘     └──────┬───────┘
           │                    │
           └────────┬───────────┘
                    │
                    ▼
              ┌───────────┐
              │   HKDF    │
              │  Combine  │
              └─────┬─────┘
                    │
                    ▼
              ┌───────────┐
              │ Final Key │ ──────▶ AES-256-GCM Encrypt
              │  (256-bit)│
              └───────────┘
                    
    Output:
    ┌─────────────────────────────────────────────────────┐
    │ Kyber ciphertext │ Salt │ Nonce │ Ciphertext │ Tag │
    │    (1568 B)      │ 32B  │  12B  │  variable  │ 16B │
    └─────────────────────────────────────────────────────┘
```

### Key Combination

```cpp
// Combine Kyber shared secret + password-derived key
std::vector<uint8_t> combine_keys(
    const std::vector<uint8_t>& kyber_secret,
    const std::vector<uint8_t>& password_key) 
{
    // Use HKDF to combine both keys
    Botan::HKDF hkdf(Botan::MessageAuthenticationCode::create("HMAC(SHA-256)"));
    
    // Concatenate both secrets as IKM
    std::vector<uint8_t> ikm;
    ikm.insert(ikm.end(), kyber_secret.begin(), kyber_secret.end());
    ikm.insert(ikm.end(), password_key.begin(), password_key.end());
    
    // Derive final 256-bit key
    Botan::secure_vector<uint8_t> final_key(32);
    hkdf.derive_key(
        final_key.data(), final_key.size(),
        ikm.data(), ikm.size(),
        nullptr, 0,  // No salt (already random)
        "FileVault-Kyber-Hybrid-v1"  // Info string
    );
    
    return std::vector<uint8_t>(final_key.begin(), final_key.end());
}
```

---

## 💻 Implement trong FileVault

### Header

```cpp
// include/filevault/algorithms/pqc/post_quantum.hpp

class Kyber : public core::CryptoAlgorithm {
public:
    enum class Variant {
        Kyber512,   // NIST Level 1 (128-bit)
        Kyber768,   // NIST Level 3 (192-bit)
        Kyber1024   // NIST Level 5 (256-bit)
    };
    
    explicit Kyber(Variant variant = Variant::Kyber768);
    
    // Key generation
    PQKeyPair generate_keypair();
    
    // KEM operations
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    // Size information
    size_t public_key_size() const;
    size_t private_key_size() const;
    size_t ciphertext_size() const;
    size_t shared_secret_size() const { return 32; }
    
private:
    Variant variant_;
};
```

### Key Generation

```cpp
PQKeyPair Kyber::generate_keypair() {
    PQKeyPair result;
    result.algorithm = name();
    
    Botan::AutoSeeded_RNG rng;
    
    // Get Botan KyberMode from variant
    Botan::KyberMode mode = get_kyber_mode(variant_);
    
    // Generate key pair
    Botan::Kyber_PrivateKey private_key(rng, mode);
    
    // Extract raw key bits
    auto priv_bits = private_key.raw_private_key_bits();
    auto pub_bits = private_key.public_key_bits();
    
    result.private_key.assign(priv_bits.begin(), priv_bits.end());
    result.public_key.assign(pub_bits.begin(), pub_bits.end());
    
    spdlog::info("Generated {} keypair: pub={} B, priv={} B",
                 name(), result.public_key.size(), 
                 result.private_key.size());
    
    return result;
}
```

### Encapsulation (Encrypt)

```cpp
core::CryptoResult Kyber::encrypt(
    std::span<const uint8_t> /* plaintext */,  // Ignored for KEM
    std::span<const uint8_t> public_key,
    const core::EncryptionConfig& /* config */) 
{
    core::CryptoResult result;
    
    Botan::AutoSeeded_RNG rng;
    Botan::KyberMode mode = get_kyber_mode(variant_);
    
    // Load public key
    std::vector<uint8_t> pk_vec(public_key.begin(), public_key.end());
    Botan::Kyber_PublicKey pub_key(pk_vec, mode);
    
    // Create encapsulator
    Botan::PK_KEM_Encryptor encryptor(pub_key, "Raw");
    
    // Encapsulate → (ciphertext, shared_secret)
    auto kem_result = encryptor.encrypt(rng, 32);
    
    // Store ciphertext
    const auto& encap_key = kem_result.encapsulated_shared_key();
    result.data.assign(encap_key.begin(), encap_key.end());
    
    // Store shared secret (repurpose nonce field)
    const auto& shared = kem_result.shared_key();
    result.nonce = std::vector<uint8_t>(shared.begin(), shared.end());
    
    result.success = true;
    return result;
}
```

---

## 🔐 Security Levels

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Kyber Security Levels                            │
└─────────────────────────────────────────────────────────────────────┘

             ┌───────────────────────────────────────────────────────┐
             │  NIST Level │ Classical  │  Quantum   │   Kyber      │
             ├─────────────┼────────────┼────────────┼──────────────┤
             │  Level 1    │  AES-128   │  128-bit   │  Kyber-512   │
             │  Level 3    │  AES-192   │  192-bit   │  Kyber-768   │
             │  Level 5    │  AES-256   │  256-bit   │  Kyber-1024  │
             └───────────────────────────────────────────────────────┘

Khuyến nghị FileVault:
├── Default: Kyber-768 (balanced)
├── Maximum: Kyber-1024 (paranoid level)
└── Minimum: Kyber-512 (still quantum-safe!)
```

---

## 🧪 Test Vectors

### Kyber-768 Test (Botan)

```cpp
TEST(Kyber, EncapsDecaps) {
    Kyber kyber(Kyber::Variant::Kyber768);
    
    // Generate keypair
    auto keypair = kyber.generate_keypair();
    ASSERT_EQ(keypair.public_key.size(), 1184);
    ASSERT_EQ(keypair.private_key.size(), 2400);
    
    // Encapsulate with public key
    auto enc_result = kyber.encrypt({}, keypair.public_key, {});
    ASSERT_TRUE(enc_result.success);
    ASSERT_EQ(enc_result.data.size(), 1088);    // Ciphertext
    ASSERT_EQ(enc_result.nonce->size(), 32);    // Shared secret
    
    // Decapsulate with private key
    core::EncryptionConfig config;
    auto dec_result = kyber.decrypt(enc_result.data, keypair.private_key, config);
    ASSERT_TRUE(dec_result.success);
    
    // Shared secrets must match!
    ASSERT_EQ(dec_result.nonce.value(), enc_result.nonce.value());
}
```

---

## 📖 Ví dụ sử dụng

```bash
# Generate Kyber-1024 keypair
filevault keygen -a kyber-1024 -o quantum-key

# Output:
# quantum-key.pub  (1568 bytes)
# quantum-key.key  (3168 bytes)

# Encrypt with Kyber-Hybrid
filevault encrypt secret.txt -a kyber-1024-hybrid

# Uses:
# 1. Kyber encapsulation → shared secret
# 2. Password → Argon2id → key
# 3. Combine both → AES-256-GCM
```

---

## 📚 Tham khảo

1. **FIPS 203** - Module-Lattice-Based Key-Encapsulation Mechanism Standard
   - https://csrc.nist.gov/pubs/fips/203/final

2. **CRYSTALS-Kyber** - Algorithm Specifications
   - https://pq-crystals.org/kyber/

3. **NIST PQC** - Post-Quantum Cryptography
   - https://csrc.nist.gov/projects/post-quantum-cryptography

4. **Botan Kyber** - Implementation
   - https://botan.randombit.net/handbook/api_ref/pubkey.html#kyber

---

<div align="center">

**[← PQC README](./README.md)** | **[Dilithium →](./dilithium.md)**

</div>
