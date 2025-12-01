# 🔐 ChaCha20-Poly1305

> **Thuật toán AEAD nhanh nhất trên các thiết bị không có hardware AES.**

---

## 📋 Mục lục

1. [Tóm tắt](#-tóm-tắt)
2. [Tại sao ChaCha20?](#-tại-sao-chacha20)
3. [Cách hoạt động](#-cách-hoạt-động)
4. [ChaCha20 Quarter Round](#-chacha20-quarter-round)
5. [Poly1305 MAC](#-poly1305-mac)
6. [Implement trong FileVault](#-implement-trong-filevault)
7. [Lỗ hổng & Mitigation](#️-lỗ-hổng--mitigation)
8. [Test Vectors](#-test-vectors)
9. [So sánh với AES-GCM](#-so-sánh-với-aes-gcm)
10. [Tham khảo](#-tham-khảo)

---

## 📝 Tóm tắt

| Thuộc tính | Giá trị |
|------------|---------|
| **Tên đầy đủ** | ChaCha20-Poly1305 AEAD (IETF variant) |
| **Loại** | AEAD (Authenticated Encryption with Associated Data) |
| **Stream cipher** | ChaCha20 (256-bit key) |
| **MAC** | Poly1305 (128-bit tag) |
| **Key size** | 256 bits (32 bytes) |
| **Nonce size** | 96 bits (12 bytes) - IETF |
| **Tag size** | 128 bits (16 bytes) |
| **Tiêu chuẩn** | RFC 8439 |

---

## 💡 Tại sao ChaCha20?

```
┌─────────────────────────────────────────────────────────────────────┐
│                    AES vs ChaCha20 Performance                      │
└─────────────────────────────────────────────────────────────────────┘

                    With AES-NI          Without AES-NI
                    (Hardware)           (Software only)
                    ──────────           ──────────────
    AES-256-GCM     ██████████ Fast     ████░░░░░░ Slow
    ChaCha20        ████████░░ Fast     ██████████ Fast!

Khi nào dùng ChaCha20:
├── Mobile devices (ARM không có crypto extensions)
├── IoT, embedded systems
├── Servers xử lý nhiều connections (TLS)
└── Khi muốn tránh side-channel attacks trên AES
```

### Design Goals

```
Daniel J. Bernstein thiết kế ChaCha20 với mục tiêu:

1. ⚡ NHANH trong software (không cần hardware support)
2. 🛡️ RESISTANT với timing attacks 
3. 🔧 ĐƠN GIẢN để implement đúng
4. 📐 DỰA TRÊN toán học vững chắc (ARX operations)
```

---

## ⚙️ Cách hoạt động

### ChaCha20-Poly1305 Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                   ChaCha20-Poly1305 AEAD                            │
└─────────────────────────────────────────────────────────────────────┘

    Key (256-bit)        Nonce (96-bit)
         │                    │
         ▼                    ▼
    ┌─────────────────────────────────┐
    │         ChaCha20 Block          │
    │    (Generate keystream)         │
    └──────────────┬──────────────────┘
                   │
       ┌───────────┴───────────┐
       │                       │
       ▼                       ▼
  ┌─────────┐            ┌─────────────┐
  │Block 0  │            │ Block 1,2.. │
  │→Poly key│            │ → Keystream │
  └────┬────┘            └──────┬──────┘
       │                        │
       │                        ▼
       │                 Plaintext ⊕ Keystream
       │                        │
       │                        ▼
       │                   Ciphertext
       │                        │
       ▼                        │
  ┌─────────┐                   │
  │Poly1305 │◄──────────────────┘
  │  MAC    │◄── AAD
  └────┬────┘
       │
       ▼
  ┌─────────┐
  │Auth Tag │
  │(128-bit)│
  └─────────┘
```

### ChaCha20 State Matrix

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ChaCha20 State (512 bits)                        │
└─────────────────────────────────────────────────────────────────────┘

    ChaCha20 state là ma trận 4x4 của 32-bit words:

    ┌──────────┬──────────┬──────────┬──────────┐
    │  const   │  const   │  const   │  const   │  ← "expand 32-byte k"
    ├──────────┼──────────┼──────────┼──────────┤
    │  key[0]  │  key[1]  │  key[2]  │  key[3]  │  ← Key (256-bit)
    ├──────────┼──────────┼──────────┼──────────┤
    │  key[4]  │  key[5]  │  key[6]  │  key[7]  │  ← Key (continued)
    ├──────────┼──────────┼──────────┼──────────┤
    │ counter  │ nonce[0] │ nonce[1] │ nonce[2] │  ← Counter + Nonce
    └──────────┴──────────┴──────────┴──────────┘
    
    Constants (ASCII): "expa" "nd 3" "2-by" "te k"
    Hex: 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
```

---

## 🔄 ChaCha20 Quarter Round

### ARX Operations (Add-Rotate-XOR)

ChaCha20 chỉ dùng 3 operations **constant-time**:

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ARX Operations                               │
└─────────────────────────────────────────────────────────────────────┘

    ⊞  Addition (mod 2³²)     - Không có timing variation
    ⊕  XOR                    - Không có timing variation  
    <<<  Left Rotation        - Không có timing variation

    → Không có table lookups như AES (S-box)
    → IMMUNE với cache-timing attacks!
```

### Quarter Round Function

```
┌─────────────────────────────────────────────────────────────────────┐
│                    QUARTERROUND(a, b, c, d)                         │
└─────────────────────────────────────────────────────────────────────┘

    def quarter_round(a, b, c, d):
        a = (a + b) mod 2³²;  d = (d ⊕ a) <<< 16
        c = (c + d) mod 2³²;  b = (b ⊕ c) <<< 12
        a = (a + b) mod 2³²;  d = (d ⊕ a) <<< 8
        c = (c + d) mod 2³²;  b = (b ⊕ c) <<< 7
        return (a, b, c, d)
    
    Visualization:
    
         a ────⊞────────────────────────⊞────────────────▶ a'
              ↑                         ↑
         b ───┼──⊞──────────────────────┼──⊞─────────────▶ b'
              │  ↑                      │  ↑
         c ───┼──┼──────────⊞───────────┼──┼──────⊞──────▶ c'
              │  │          ↑           │  │      ↑
         d ───┼──┼──────────┼──⊕──<<<16─┼──┼──────┼──⊕──<<<8──▶ d'
              │  │          │  │        │  │      │  │
              │  └──⊕──<<<12┘  │        │  └─⊕<<<7┘  │
              └────────────────┘        └────────────┘
```

### Full ChaCha20 Block Function

```python
def chacha20_block(key, counter, nonce):
    """Generate 64 bytes of keystream"""
    
    # Initialize state matrix
    state = [
        0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,  # Constants
        key[0],     key[1],     key[2],     key[3],      # Key
        key[4],     key[5],     key[6],     key[7],      # Key
        counter,    nonce[0],   nonce[1],   nonce[2]     # Counter + Nonce
    ]
    
    working_state = state.copy()
    
    # 20 rounds (10 column rounds + 10 diagonal rounds)
    for _ in range(10):
        # Column rounds
        quarter_round(working_state, 0, 4,  8, 12)
        quarter_round(working_state, 1, 5,  9, 13)
        quarter_round(working_state, 2, 6, 10, 14)
        quarter_round(working_state, 3, 7, 11, 15)
        
        # Diagonal rounds  
        quarter_round(working_state, 0, 5, 10, 15)
        quarter_round(working_state, 1, 6, 11, 12)
        quarter_round(working_state, 2, 7,  8, 13)
        quarter_round(working_state, 3, 4,  9, 14)
    
    # Add original state
    output = [(working_state[i] + state[i]) & 0xFFFFFFFF 
              for i in range(16)]
    
    return serialize_le(output)  # 64 bytes keystream
```

### Round Visualization

```
    Column Rounds               Diagonal Rounds
    ─────────────               ───────────────
    
    0  1  2  3                  0  1  2  3
    ↓  ↓  ↓  ↓                   ╲ ╲ ╲ ╲
    4  5  6  7                  4  5  6  7
    ↓  ↓  ↓  ↓                    ╲ ╲ ╲ ╲
    8  9  10 11                 8  9  10 11
    ↓  ↓  ↓  ↓                     ╲ ╲ ╲ ╲
    12 13 14 15                 12 13 14 15
    
    QR(0,4,8,12)                QR(0,5,10,15)
    QR(1,5,9,13)                QR(1,6,11,12)
    QR(2,6,10,14)               QR(2,7,8,13)
    QR(3,7,11,15)               QR(3,4,9,14)
```

---

## 🔏 Poly1305 MAC

### Poly1305 Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Poly1305 MAC                                 │
└─────────────────────────────────────────────────────────────────────┘

Poly1305 tính authentication tag bằng polynomial evaluation:

    Tag = ((c₁·r^n + c₂·r^(n-1) + ... + cₙ·r¹) mod p) + s) mod 2¹²⁸

Với:
├── r: clamped 128-bit key (từ ChaCha20 block 0, bytes 0-15)
├── s: 128-bit key (từ ChaCha20 block 0, bytes 16-31)
├── p: prime = 2¹³⁰ - 5
├── cᵢ: message blocks (128-bit chunks + length byte)
└── n: số blocks
```

### Poly1305 Key Derivation

```
┌─────────────────────────────────────────────────────────────────────┐
│               Poly1305 Key from ChaCha20                            │
└─────────────────────────────────────────────────────────────────────┘

    ChaCha20(key, nonce, counter=0) → 64 bytes
    
    ┌────────────────────────────────────────────────────────────┐
    │  Block 0 output (64 bytes)                                 │
    ├────────────────────┬───────────────────┬───────────────────┤
    │     r (16 bytes)   │    s (16 bytes)   │  (unused 32 B)    │
    │    Poly1305 r      │   Poly1305 s      │                   │
    └────────────────────┴───────────────────┴───────────────────┘
    
    r clamping (để đảm bảo fast reduction mod p):
    r[3], r[7], r[11], r[15] &= 0x0F   # Clear top 4 bits
    r[4], r[8], r[12]        &= 0xFC   # Clear bottom 2 bits
```

### Poly1305 Computation

```python
def poly1305_mac(key_r, key_s, message):
    """Compute Poly1305 MAC"""
    
    r = clamp(key_r)
    s = key_s
    p = (1 << 130) - 5
    
    accumulator = 0
    
    for block in pad_and_split(message, 16):
        # Add high bit to prevent length extension
        n = bytes_to_int_le(block) | (1 << (8 * len(block)))
        
        # Accumulate
        accumulator = ((accumulator + n) * r) % p
    
    # Final: add s
    tag = (accumulator + s) % (1 << 128)
    
    return int_to_bytes_le(tag, 16)
```

---

## 💻 Implement trong FileVault

### Header

```cpp
// include/filevault/algorithms/symmetric/chacha20_poly1305.hpp

class ChaCha20Poly1305 : public core::CryptoAlgorithm {
public:
    ChaCha20Poly1305();
    
    std::string name() const override { return "ChaCha20-Poly1305"; }
    core::AlgorithmType type() const override;
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    // Fixed sizes
    size_t key_size() const override { return 32; }  // 256 bits
    size_t nonce_size() const { return 12; }         // 96 bits (IETF)
    size_t tag_size() const { return 16; }           // 128 bits
};
```

### Implementation

```cpp
// src/algorithms/symmetric/chacha20_poly1305.cpp

core::CryptoResult ChaCha20Poly1305::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) 
{
    core::CryptoResult result;
    
    // ═══════════════════════════════════════════════════════════
    // STEP 1: Validate key (must be 256-bit)
    // ═══════════════════════════════════════════════════════════
    if (key.size() != 32) {
        result.success = false;
        result.error_message = "Key must be 32 bytes";
        return result;
    }
    
    // ═══════════════════════════════════════════════════════════
    // STEP 2: Generate unique 96-bit nonce
    // ═══════════════════════════════════════════════════════════
    std::vector<uint8_t> nonce;
    if (config.nonce.has_value()) {
        nonce = config.nonce.value();  // Testing
    } else {
        Botan::AutoSeeded_RNG rng;
        nonce.resize(12);
        rng.randomize(nonce.data(), nonce.size());
    }
    
    // ═══════════════════════════════════════════════════════════
    // STEP 3: Create AEAD cipher (Botan uses IETF variant)
    // ═══════════════════════════════════════════════════════════
    auto cipher = Botan::AEAD_Mode::create(
        "ChaCha20Poly1305", 
        Botan::Cipher_Dir::Encryption
    );
    
    cipher->set_key(key.data(), key.size());
    
    // Optional: Associated Data
    if (config.associated_data.has_value()) {
        const auto& ad = config.associated_data.value();
        cipher->set_associated_data(ad.data(), ad.size());
    }
    
    // ═══════════════════════════════════════════════════════════
    // STEP 4: Encrypt
    // ═══════════════════════════════════════════════════════════
    cipher->start(nonce.data(), nonce.size());
    
    Botan::secure_vector<uint8_t> buffer(
        plaintext.begin(), plaintext.end()
    );
    cipher->finish(buffer);
    
    // ═══════════════════════════════════════════════════════════
    // STEP 5: Split ciphertext and tag
    // ═══════════════════════════════════════════════════════════
    size_t ct_len = buffer.size() - 16;  // Tag is 16 bytes
    
    result.data.assign(buffer.begin(), buffer.begin() + ct_len);
    result.tag = std::vector<uint8_t>(
        buffer.begin() + ct_len, buffer.end()
    );
    result.nonce = nonce;
    result.success = true;
    
    return result;
}
```

---

## ⚠️ Lỗ hổng & Mitigation

### 1. Nonce Reuse (Same as GCM!)

```
⚠️ CRITICAL: ChaCha20-Poly1305 cũng bị ảnh hưởng bởi nonce reuse!

Nếu (Key, Nonce) được dùng lại:
├── Keystream lặp lại → XOR recovery attack
├── Poly1305 key (r,s) lặp lại → Forgery possible
└── Toàn bộ bảo mật bị phá vỡ!

Mitigation:
├── Random nonce (96-bit): OK cho ~2^48 messages/key
├── Counter nonce: Đảm bảo unique
└── XChaCha20-Poly1305: 192-bit nonce (an toàn hơn với random)
```

### 2. Timing Attacks (Already Mitigated!)

```
ChaCha20 được thiết kế để RESISTANT với timing attacks:

┌────────────────────────────────────────────────────────────┐
│  Operation    │  AES (S-box)  │  ChaCha20 (ARX)           │
├───────────────┼───────────────┼────────────────────────────┤
│  Table lookup │      ✓        │       ✗ (không có)        │
│  Cache timing │   Vulnerable  │       Immune              │
│  Constant-time│   Khó impl    │       Tự nhiên            │
└───────────────┴───────────────┴────────────────────────────┘
```

---

## 🧪 Test Vectors

### RFC 8439 Test Vector

```
ChaCha20-Poly1305 AEAD Test Vector
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Key (256-bit):
  808182838485868788898a8b8c8d8e8f
  909192939495969798999a9b9c9d9e9f

Nonce (96-bit):
  070000004041424344454647

AAD (Associated Data):
  50515253c0c1c2c3c4c5c6c7

Plaintext:
  "Ladies and Gentlemen of the class of '99: 
   If I could offer you only one tip for the future, 
   sunscreen would be it."

Expected Ciphertext:
  d31a8d34648e60db7b86afbc53ef7ec2
  a4aded51296e08fea9e2b5a736ee62d6
  3dbea45e8ca9671282fafb69da92728b
  1a71de0a9e060b2905d6a5b67ecd3b36
  92ddbd7f2d778b8c9803aee328091b58
  fab324e4fad675945585808b4831d7bc
  3ff4def08e4b7a9de576d26586cec64b
  6116

Expected Tag:
  1ae10b594f09e26a7e902ecbd0600691
```

---

## 📊 So sánh với AES-GCM

```
┌─────────────────────────────────────────────────────────────────────┐
│              ChaCha20-Poly1305 vs AES-256-GCM                       │
└─────────────────────────────────────────────────────────────────────┘

                    ChaCha20-Poly1305      AES-256-GCM
                    ─────────────────      ───────────
Key size            256-bit                256-bit
Nonce size          96-bit                 96-bit  
Tag size            128-bit                128-bit
Block size          512-bit (stream)       128-bit

Performance:
  With AES-NI       ████████░░             ██████████
  Without AES-NI    ██████████             ████░░░░░░
  Mobile (ARM)      ██████████             ██████░░░░

Security:
  Timing attacks    Immune ✓               Needs care
  Side-channels     Resistant              Table lookups
  Nonce misuse      Catastrophic           Catastrophic

Adoption:
  TLS 1.3           Mandatory              Mandatory
  WireGuard         Primary choice         Not used
  SSH               Supported              Supported
```

### Benchmark trên FileVault

```
$ filevault benchmark --symmetric

┌─────────────────────────────────────────────────────────────┐
│           Symmetric Encryption Benchmark (10MB file)        │
├─────────────────────┬───────────────┬───────────────────────┤
│      Algorithm      │    Speed      │       Notes           │
├─────────────────────┼───────────────┼───────────────────────┤
│ AES-256-GCM         │  2.1 GB/s     │ With AES-NI           │
│ ChaCha20-Poly1305   │  1.8 GB/s     │ Pure software         │
│ Serpent-256-GCM     │  450 MB/s     │ Conservative          │
└─────────────────────┴───────────────┴───────────────────────┘
```

---

## 📖 Ví dụ sử dụng

```bash
# Mã hóa với ChaCha20-Poly1305
filevault encrypt video.mp4 -a chacha20-poly1305

# Phù hợp cho mobile/IoT devices không có AES hardware
filevault encrypt sensor_data.bin -a chacha20-poly1305 -s medium
```

---

## 📚 Tham khảo

1. **RFC 8439** - ChaCha20 and Poly1305 for IETF Protocols
   - https://tools.ietf.org/html/rfc8439

2. **"ChaCha, a variant of Salsa20"** - Daniel J. Bernstein
   - https://cr.yp.to/chacha.html

3. **"The Poly1305-AES message-authentication code"** - D.J. Bernstein
   - https://cr.yp.to/mac.html

4. **Botan ChaCha20-Poly1305**
   - https://botan.randombit.net/handbook/api_ref/aead.html

---

<div align="center">

**[← AES-GCM](./aes-gcm.md)** | **[Symmetric README](./README.md)** | **[Serpent-GCM →](./serpent-gcm.md)**

</div>
