# 🔐 AES-GCM (Advanced Encryption Standard - Galois/Counter Mode)

> **Thuật toán mã hóa đối xứng AEAD được khuyến nghị cho hầu hết các ứng dụng.**

---

## 📋 Mục lục

1. [Tóm tắt](#-tóm-tắt)
2. [Lý thuyết](#-lý-thuyết)
3. [Cách hoạt động](#-cách-hoạt-động)
4. [Cấu trúc dữ liệu](#-cấu-trúc-dữ-liệu)
5. [Implement trong FileVault](#-implement-trong-filevault)
6. [Lỗ hổng & Mitigation](#️-lỗ-hổng--mitigation)
7. [Test Vectors](#-test-vectors)
8. [Ví dụ sử dụng](#-ví-dụ-sử-dụng)
9. [So sánh với các mode khác](#-so-sánh-với-các-mode-khác)
10. [Tham khảo](#-tham-khảo)

---

## 📝 Tóm tắt

| Thuộc tính | Giá trị |
|------------|---------|
| **Tên đầy đủ** | Advanced Encryption Standard - Galois/Counter Mode |
| **Loại** | AEAD (Authenticated Encryption with Associated Data) |
| **Block size** | 128 bits (16 bytes) |
| **Key sizes** | 128, 192, 256 bits |
| **Nonce size** | 96 bits (12 bytes) - khuyến nghị |
| **Tag size** | 128 bits (16 bytes) |
| **Tiêu chuẩn** | NIST SP 800-38D |

### Ưu điểm
- ✅ **AEAD**: Vừa mã hóa vừa xác thực trong một bước
- ✅ **Nhanh**: Hỗ trợ hardware acceleration (AES-NI)
- ✅ **An toàn**: Được NIST chứng nhận, dùng rộng rãi (TLS 1.3)
- ✅ **Parallel**: Có thể mã hóa song song

### Nhược điểm
- ⚠️ **Nonce reuse catastrophic**: Dùng lại nonce = mất toàn bộ bảo mật
- ⚠️ **Giới hạn dữ liệu**: Tối đa 64GB per key/nonce pair

---

## 📐 Lý thuyết

### AES (Block Cipher)

AES là **block cipher** biến đổi 128-bit plaintext thành 128-bit ciphertext sử dụng key.

```
┌─────────────────────────────────────────────────────────────┐
│                     AES Round (x10/12/14)                   │
├─────────────────────────────────────────────────────────────┤
│  Plaintext (128-bit)                                        │
│       ↓                                                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │ SubBytes    │→ │ ShiftRows   │→ │ MixColumns  │→ AddKey │
│  │ (S-box)     │  │ (Rotate)    │  │ (Matrix)    │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
│       ↓                                                     │
│  Ciphertext (128-bit)                                       │
└─────────────────────────────────────────────────────────────┘
```

**Số rounds theo key size:**
- AES-128: 10 rounds
- AES-192: 12 rounds  
- AES-256: 14 rounds

### GCM (Galois/Counter Mode)

GCM kết hợp **CTR mode** (mã hóa) với **GHASH** (xác thực).

```
                        ┌──────────────────────────────────────┐
                        │            GCM Structure             │
                        └──────────────────────────────────────┘
                                        
    Nonce (96-bit)          Key (128/256-bit)
         │                        │
         ▼                        ▼
    ┌─────────┐              ┌─────────┐
    │ Counter │─────────────▶│   AES   │──────▶ Keystream
    │  (32-bit)│              │  Block  │
    └─────────┘              └─────────┘
         │                                         │
         │    ┌────────────────────────────────────┘
         │    │
         │    ▼
         │  Plaintext ⊕ Keystream = Ciphertext
         │                              │
         │                              ▼
         │                        ┌─────────┐
         └───────────────────────▶│  GHASH  │──────▶ Auth Tag
              AAD ───────────────▶│         │
                                  └─────────┘
```

### GHASH Function

GHASH là phép nhân trong **Galois Field GF(2^128)**:

$$GHASH(H, A, C) = X_m$$

Với:
- $H = E_K(0^{128})$ (hash key)
- $A$ = Associated Data
- $C$ = Ciphertext
- Phép nhân: $X_i = (X_{i-1} \oplus A_i) \cdot H$ trong GF(2^128)

---

## ⚙️ Cách hoạt động

### Encryption Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         AES-GCM ENCRYPTION                              │
└─────────────────────────────────────────────────────────────────────────┘

  INPUT                           PROCESS                        OUTPUT
  ─────                           ───────                        ──────

┌──────────┐                                                   ┌──────────┐
│ Plaintext│                                                   │Ciphertext│
│  (P)     │                                                   │   (C)    │
└────┬─────┘                                                   └────▲─────┘
     │                                                              │
     │    ┌─────────────────────────────────────────────────────────┤
     │    │                                                         │
     ▼    │                                                         │
┌────────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐        │
│   Nonce    │───▶│Counter=1│───▶│   AES   │───▶│Keystream│────────┘
│ (96-bit)   │    │ (32-bit)│    │         │    │         │    XOR (⊕)
└────────────┘    └─────────┘    └─────────┘    └─────────┘
     │                                                         
     │            ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌────────┐
     └───────────▶│Counter=0│───▶│   AES   │───▶│  H key  │───▶│ GHASH  │
                  └─────────┘    └─────────┘    └─────────┘    └───┬────┘
                                                                   │
┌────────────┐                                                     │
│    AAD     │─────────────────────────────────────────────────────┘
│(Associated │                                                     │
│   Data)    │                                                     ▼
└────────────┘                                              ┌──────────┐
                                                            │Auth Tag  │
                                                            │(128-bit) │
                                                            └──────────┘
```

### Step-by-step Encryption

```python
def aes_gcm_encrypt(plaintext, key, nonce, aad=""):
    """
    Step 1: Generate hash key H
    """
    H = AES_encrypt(key, zeros(128))
    
    """
    Step 2: Initialize counter
    """
    if len(nonce) == 96:
        J0 = nonce || 0x00000001  # 96-bit nonce + 32-bit counter
    else:
        J0 = GHASH(H, "", nonce)  # Handle other nonce sizes
    
    """
    Step 3: Encrypt plaintext using CTR mode
    """
    ciphertext = ""
    counter = increment(J0)  # Start from J0 + 1
    
    for block in split_blocks(plaintext, 128):
        keystream = AES_encrypt(key, counter)
        ciphertext += block XOR keystream
        counter = increment(counter)
    
    """
    Step 4: Calculate authentication tag
    """
    # GHASH over AAD and Ciphertext
    S = GHASH(H, aad, ciphertext)
    
    # Final tag = S XOR E(K, J0)
    tag = S XOR AES_encrypt(key, J0)
    
    return (ciphertext, tag)
```

### Decryption Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         AES-GCM DECRYPTION                              │
└─────────────────────────────────────────────────────────────────────────┘

  Step 1: Verify Tag First!
  ─────────────────────────
  
  ┌──────────┐     ┌──────────┐     ┌─────────────┐
  │Ciphertext│────▶│  GHASH   │────▶│Computed Tag │
  │   + AAD  │     │          │     └──────┬──────┘
  └──────────┘     └──────────┘            │
                                           │ Compare
  ┌──────────┐                             │ (constant-time!)
  │Received  │─────────────────────────────┘
  │   Tag    │            │
  └──────────┘            ▼
                    ┌──────────┐
                    │  Match?  │
                    └────┬─────┘
                         │
           ┌─────────────┴─────────────┐
           │                           │
           ▼                           ▼
    ┌────────────┐              ┌────────────┐
    │    YES     │              │     NO     │
    │  Decrypt   │              │   REJECT   │
    │ Ciphertext │              │  (Error!)  │
    └────────────┘              └────────────┘
```

---

## 📊 Cấu trúc dữ liệu

### Key Format

```
┌────────────────────────────────────────────────────────┐
│                    AES Key                             │
├────────────────────────────────────────────────────────┤
│  AES-128:  ████████████████  (16 bytes = 128 bits)     │
│  AES-192:  ████████████████████████  (24 bytes)        │
│  AES-256:  ████████████████████████████████  (32 bytes)│
└────────────────────────────────────────────────────────┘

⚠️ Key PHẢI được generate từ CSPRNG!
   Trong FileVault: Botan::AutoSeeded_RNG
```

### Nonce Format (96-bit recommended)

```
┌─────────────────────────────────────────────────────────┐
│                    96-bit Nonce                         │
├─────────────────────────────────────────────────────────┤
│  ████████████████████████████████████████████████████   │
│  └──────────── 12 bytes (96 bits) ─────────────────┘    │
│                                                         │
│  Counter format (internal):                             │
│  ┌──────────────────────────┬─────────────┐            │
│  │    Nonce (96 bits)       │ Counter(32) │            │
│  └──────────────────────────┴─────────────┘            │
│                               ↑                        │
│                    Starts at 1, increments             │
└─────────────────────────────────────────────────────────┘

⚠️ QUAN TRỌNG: Mỗi encryption PHẢI dùng nonce MỚI!
   - Random: Sử dụng CSPRNG
   - Counter: Tăng dần, lưu state
   - NEVER reuse nonce với cùng key!
```

### Output Format (FileVault .fvlt)

```
┌───────────────────────────────────────────────────────────────────┐
│                    Encrypted File Structure                        │
├───────────────────────────────────────────────────────────────────┤
│  ┌──────────────┬──────────────┬──────────────┬────────────────┐  │
│  │  File Header │    Nonce     │  Ciphertext  │      Tag       │  │
│  │   (metadata) │  (12 bytes)  │  (variable)  │   (16 bytes)   │  │
│  └──────────────┴──────────────┴──────────────┴────────────────┘  │
│                                                                    │
│  Header contains:                                                  │
│  - Magic bytes ("FVLT")                                           │
│  - Version                                                         │
│  - Algorithm ID (AES-256-GCM = 0x03)                              │
│  - Salt (for key derivation)                                       │
│  - Original filename (encrypted)                                   │
└───────────────────────────────────────────────────────────────────┘
```

---

## 💻 Implement trong FileVault

### Header (aes_gcm.hpp)

```cpp
// include/filevault/algorithms/symmetric/aes_gcm.hpp

class AES_GCM : public core::CryptoAlgorithm {
public:
    explicit AES_GCM(size_t key_bits = 256);
    
    // Interface methods
    std::string name() const override;
    core::AlgorithmType type() const override;
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    // Properties
    size_t key_size() const override { return key_bits_ / 8; }
    size_t nonce_size() const { return 12; }  // 96 bits
    size_t tag_size() const { return 16; }    // 128 bits
    
private:
    size_t key_bits_;
    core::AlgorithmType type_;
    std::string botan_name_;
};
```

### Implementation (aes_gcm.cpp) - Key Points

```cpp
// src/algorithms/symmetric/aes_gcm.cpp

core::CryptoResult AES_GCM::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) 
{
    core::CryptoResult result;
    
    // ═══════════════════════════════════════════════════════════
    // STEP 1: Validate key size
    // ═══════════════════════════════════════════════════════════
    if (key.size() != key_size()) {
        result.success = false;
        result.error_message = "Invalid key size";
        return result;
    }
    
    // ═══════════════════════════════════════════════════════════
    // STEP 2: Generate unique nonce (CRITICAL SECURITY!)
    // ═══════════════════════════════════════════════════════════
    std::vector<uint8_t> nonce;
    
    if (config.nonce.has_value()) {
        // Testing mode only
        nonce = config.nonce.value();
    } else {
        // PRODUCTION: Always generate new random nonce
        Botan::AutoSeeded_RNG rng;
        nonce.resize(nonce_size());      // 12 bytes
        rng.randomize(nonce.data(), nonce.size());
    }
    
    // ═══════════════════════════════════════════════════════════
    // STEP 3: Create AEAD cipher (Botan)
    // ═══════════════════════════════════════════════════════════
    auto cipher = Botan::AEAD_Mode::create(
        botan_name_,                    // "AES-256/GCM"
        Botan::Cipher_Dir::Encryption
    );
    
    // ═══════════════════════════════════════════════════════════
    // STEP 4: Set key and AAD
    // ═══════════════════════════════════════════════════════════
    cipher->set_key(key.data(), key.size());
    
    if (config.associated_data.has_value()) {
        const auto& ad = config.associated_data.value();
        cipher->set_associated_data(ad.data(), ad.size());
    }
    
    // ═══════════════════════════════════════════════════════════
    // STEP 5: Encrypt
    // ═══════════════════════════════════════════════════════════
    cipher->start(nonce.data(), nonce.size());
    
    Botan::secure_vector<uint8_t> buffer(
        plaintext.begin(), plaintext.end()
    );
    cipher->finish(buffer);  // Ciphertext + Tag appended
    
    // ═══════════════════════════════════════════════════════════
    // STEP 6: Separate ciphertext and tag
    // ═══════════════════════════════════════════════════════════
    size_t ct_len = buffer.size() - tag_size();
    
    result.data.assign(buffer.begin(), buffer.begin() + ct_len);
    result.tag = std::vector<uint8_t>(
        buffer.begin() + ct_len, buffer.end()
    );
    result.nonce = nonce;
    result.success = true;
    
    return result;
}
```

### Luồng Giải mã

```cpp
core::CryptoResult AES_GCM::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) 
{
    // ═══════════════════════════════════════════════════════════
    // STEP 1: Get nonce and tag from config
    // ═══════════════════════════════════════════════════════════
    if (!config.nonce.has_value() || !config.tag.has_value()) {
        return {.success = false, 
                .error_message = "Nonce and tag required"};
    }
    
    auto& nonce = config.nonce.value();
    auto& tag = config.tag.value();
    
    // ═══════════════════════════════════════════════════════════
    // STEP 2: Create cipher and set parameters
    // ═══════════════════════════════════════════════════════════
    auto cipher = Botan::AEAD_Mode::create(
        botan_name_, 
        Botan::Cipher_Dir::Decryption
    );
    
    cipher->set_key(key.data(), key.size());
    cipher->start(nonce.data(), nonce.size());
    
    // ═══════════════════════════════════════════════════════════
    // STEP 3: Combine ciphertext + tag, then decrypt
    // ═══════════════════════════════════════════════════════════
    Botan::secure_vector<uint8_t> buffer;
    buffer.insert(buffer.end(), ciphertext.begin(), ciphertext.end());
    buffer.insert(buffer.end(), tag.begin(), tag.end());
    
    // ⚠️ finish() sẽ VERIFY tag trước khi trả về plaintext
    // Nếu tag sai → throw exception
    cipher->finish(buffer);
    
    result.data.assign(buffer.begin(), buffer.end());
    result.success = true;
    
    return result;
}
```

---

## ⚠️ Lỗ hổng & Mitigation

### 1. Nonce Reuse Attack (CRITICAL!)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ⚠️ NONCE REUSE CATASTROPHE                       │
└─────────────────────────────────────────────────────────────────────┘

Nếu dùng CÙNG (Key, Nonce) để mã hóa 2 plaintext khác nhau:

    P1 ⊕ Keystream = C1
    P2 ⊕ Keystream = C2
    ─────────────────────
    C1 ⊕ C2 = P1 ⊕ P2    ← Attacker có thể XOR để lộ plaintext!

Hậu quả:
├── Mất tính bảo mật (Confidentiality)
├── Authentication key (H) bị lộ
└── Attacker có thể forge messages!
```

**Mitigation trong FileVault:**

```cpp
// LUÔN generate nonce mới cho mỗi encryption
Botan::AutoSeeded_RNG rng;
nonce.resize(12);
rng.randomize(nonce.data(), nonce.size());

// Với 96-bit random nonce:
// P(collision) < 2^-32 sau 2^32 encryptions (Birthday bound)
// → Vẫn an toàn cho ~4 billion encryptions per key
```

### 2. Authentication Tag Bypass

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ⚠️ TAG VERIFICATION                              │
└─────────────────────────────────────────────────────────────────────┘

WRONG ❌                           RIGHT ✓
──────────                         ─────────
1. Decrypt                         1. Compute expected tag
2. Verify tag                      2. Compare tags (constant-time!)
3. Use plaintext                   3. If match → decrypt
                                   4. If not → REJECT immediately

// Botan's AEAD_Mode::finish() does this correctly!
```

**Constant-time comparison (Botan internal):**

```cpp
// Prevent timing attacks
bool tags_equal = Botan::constant_time_compare(
    computed_tag.data(),
    received_tag.data(),
    16
);
```

### 3. Data Limit

```
┌─────────────────────────────────────────────────────────────────────┐
│                    GCM Data Limits                                  │
└─────────────────────────────────────────────────────────────────────┘

Per (Key, Nonce) pair:
├── Maximum plaintext: 2^39 - 256 bits (~64 GB)
├── Maximum AAD: 2^64 - 1 bits
└── Recommended: Re-key sau 2^32 blocks (~64 GB)

FileVault mitigation:
- Mỗi file dùng unique salt → unique derived key
- Mỗi encryption dùng unique nonce
- Không stream file > 4GB trong một operation
```

---

## 🧪 Test Vectors

### NIST Test Vector (SP 800-38D)

```
Test Case 1: AES-256-GCM
━━━━━━━━━━━━━━━━━━━━━━━━

Key (256-bit):
  feffe9928665731c6d6a8f9467308308
  feffe9928665731c6d6a8f9467308308

Nonce (96-bit):
  cafebabefacedbaddecaf888

AAD:
  feedfacedeadbeeffeedfacedeadbeef
  abaddad2

Plaintext:
  d9313225f88406e5a55909c5aff5269a
  86a7a9531534f7da2e4c303d8a318a72
  1c3c0c95956809532fcf0e2449a6b525
  b16aedf5aa0de657ba637b39

Expected Ciphertext:
  522dc1f099567d07f47f37a32a84427d
  643a8cdcbfe5c0c97598a2bd2555d1aa
  8cb08e48590dbb3da7b08b1056828838
  c5f61e6393ba7a0abcc9f662

Expected Tag:
  76fc6ece0f4e1768cddf8853bb2d551b
```

### FileVault Test

```cpp
// tests/unit/crypto/test_aes_gcm.cpp

TEST(AES_GCM, EncryptDecrypt) {
    AES_GCM cipher(256);
    
    std::vector<uint8_t> key(32);
    std::vector<uint8_t> plaintext = {'H','e','l','l','o'};
    
    Botan::AutoSeeded_RNG rng;
    rng.randomize(key.data(), key.size());
    
    // Encrypt
    auto enc_result = cipher.encrypt(plaintext, key, {});
    ASSERT_TRUE(enc_result.success);
    ASSERT_EQ(enc_result.nonce->size(), 12);
    ASSERT_EQ(enc_result.tag->size(), 16);
    
    // Decrypt
    core::EncryptionConfig dec_config;
    dec_config.nonce = enc_result.nonce;
    dec_config.tag = enc_result.tag;
    
    auto dec_result = cipher.decrypt(enc_result.data, key, dec_config);
    ASSERT_TRUE(dec_result.success);
    ASSERT_EQ(dec_result.data, plaintext);
}
```

---

## 📖 Ví dụ sử dụng

### CLI Commands

```bash
# Mã hóa file với AES-256-GCM (mặc định)
filevault encrypt document.pdf -a aes-256-gcm -s medium

# Mã hóa với security level cao
filevault encrypt secret.txt -a aes-256-gcm -s paranoid

# Giải mã
filevault decrypt document.pdf.fvlt

# Xem thông tin file đã mã hóa
filevault info document.pdf.fvlt
```

### Output Example

```
$ filevault encrypt document.pdf -a aes-256-gcm -s medium

[info] Algorithm: AES-256-GCM
[info] Security Level: medium (Argon2id, 16MB, 2 iterations)
Enter password: ********
Confirm password: ********

[████████████████████████████████████████] 100%

[success] Encrypted: document.pdf → document.pdf.fvlt
[info] Original: 1.2 MB → Encrypted: 1.2 MB + 16 bytes tag
[info] Time: 45.23 ms
```

---

## 📊 So sánh với các mode khác

```
┌─────────────────┬────────┬────────┬──────────┬────────────┬──────────┐
│      Mode       │Encrypt │Decrypt │  AEAD    │ Parallel   │  Nonce   │
│                 │        │        │          │            │  Reuse   │
├─────────────────┼────────┼────────┼──────────┼────────────┼──────────┤
│ GCM             │   ✓    │   ✓    │    ✓     │ Enc+Dec    │ FATAL ☠️ │
│ CTR             │   ✓    │   ✓    │    ✗     │ Enc+Dec    │ FATAL ☠️ │
│ CBC             │   ✓    │   ✓    │    ✗     │ Dec only   │ Secure   │
│ ECB             │   ✓    │   ✓    │    ✗     │ Enc+Dec    │ N/A      │
│ OCB             │   ✓    │   ✓    │    ✓     │ Enc+Dec    │ FATAL ☠️ │
│ CCM             │   ✓    │   ✓    │    ✓     │ Neither    │ FATAL ☠️ │
└─────────────────┴────────┴────────┴──────────┴────────────┴──────────┘

Khuyến nghị:
├── Mã hóa file:     AES-256-GCM ✓
├── Disk encryption: AES-256-XTS
├── Software-only:   ChaCha20-Poly1305
└── Legacy support:  AES-256-CBC + HMAC-SHA256
```

---

## 📚 Tham khảo

1. **NIST SP 800-38D** - Recommendation for Block Cipher Modes: GCM and GMAC
   - https://csrc.nist.gov/publications/detail/sp/800-38d/final

2. **RFC 5116** - An Interface and Algorithms for Authenticated Encryption
   - https://tools.ietf.org/html/rfc5116

3. **Botan Library Documentation**
   - https://botan.randombit.net/handbook/api_ref/aead.html

4. **The Galois/Counter Mode of Operation (GCM)** - McGrew & Viega
   - Original paper describing GCM

---

<div align="center">

**[← Quay lại Symmetric](./README.md)** | **[ChaCha20-Poly1305 →](./chacha20-poly1305.md)**

</div>
