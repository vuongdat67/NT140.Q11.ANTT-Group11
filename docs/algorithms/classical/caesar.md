# 📚 Caesar Cipher - Mật mã Dịch chuyển

> **Thuật toán mật mã cổ điển đơn giản nhất - CHỈ DÀNH CHO HỌC TẬP!**

---

## ⚠️ Cảnh báo Bảo mật

```
╔══════════════════════════════════════════════════════════════════════╗
║  ⚠️  KHÔNG SỬ DỤNG CHO DỮ LIỆU THỰC TẾ!                              ║
║                                                                       ║
║  Caesar cipher có thể bị phá trong VÀI GIÂY bằng:                    ║
║  - Brute force (chỉ 26 key possibilities)                            ║
║  - Frequency analysis                                                 ║
║                                                                       ║
║  Chỉ sử dụng để: HỌC TẬP, DEMO, GIẢNG DẠY                           ║
╚══════════════════════════════════════════════════════════════════════╝
```

---

## 📋 Mục lục

1. [Lịch sử](#-lịch-sử)
2. [Cách hoạt động](#-cách-hoạt-động)
3. [Implement trong FileVault](#-implement-trong-filevault)
4. [Phương pháp tấn công](#-phương-pháp-tấn-công)
5. [Demo Brute Force](#-demo-brute-force)
6. [Bài tập thực hành](#-bài-tập-thực-hành)

---

## 📜 Lịch sử

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Julius Caesar (~100-44 BC)                       │
└─────────────────────────────────────────────────────────────────────┘

    Julius Caesar sử dụng mật mã này để liên lạc với các tướng lĩnh.
    
    Ông thường dùng SHIFT = 3:
    
    A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
    ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
    D E F G H I J K L M N O P Q R S T U V W X Y Z A B C
    
    "ATTACK" → "DWWDFN"
    
    Lý do hoạt động (tạm thời):
    ├── Hầu hết người thời đó không biết đọc
    ├── Không có máy tính để brute force
    └── "Security through obscurity" đủ dùng
```

---

## ⚙️ Cách hoạt động

### Công thức Toán học

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Caesar Cipher Formula                            │
└─────────────────────────────────────────────────────────────────────┘

    Encryption:
    ───────────
    E(x) = (x + k) mod 26
    
    Với:
    ├── x = vị trí chữ cái (A=0, B=1, ..., Z=25)
    ├── k = shift amount (key)
    └── mod 26 = wrap around sau Z
    
    Decryption:
    ───────────
    D(x) = (x - k) mod 26
    
    Hoặc:
    D(x) = (x + (26 - k)) mod 26
```

### Ví dụ Step-by-step

```
    Plaintext:  H E L L O
    Shift (k):  3
    
    ┌─────┬─────────────────────────────────────────────────┐
    │Step │ Calculation                                     │
    ├─────┼─────────────────────────────────────────────────┤
    │  H  │ Position: 7  → (7 + 3) mod 26 = 10  → K        │
    │  E  │ Position: 4  → (4 + 3) mod 26 = 7   → H        │
    │  L  │ Position: 11 → (11 + 3) mod 26 = 14 → O        │
    │  L  │ Position: 11 → (11 + 3) mod 26 = 14 → O        │
    │  O  │ Position: 14 → (14 + 3) mod 26 = 17 → R        │
    └─────┴─────────────────────────────────────────────────┘
    
    Ciphertext: K H O O R
```

### Visual Cipher Wheel

```
                        ENCRYPT →
                    ┌─────────────┐
                    │      A      │
                ┌───┴───┐   ┌───┴───┐
              Y │       │ B │       │ C
              ──┤       ├───┤       ├──
              X │       │   │       │ D
                │   ●   │   │   ●   │
              W │ INNER │   │ OUTER │ E
              ──┤       ├───┤       ├──
              V │ (k=3) │ G │       │ F
                └───┬───┘   └───┬───┘
                    │      H      │
                    └─────────────┘
                        ← DECRYPT

    Inner ring: Plaintext (A-Z)
    Outer ring: Ciphertext (shifted by k)
    
    Rotate inner ring by k positions to encrypt/decrypt
```

---

## 💻 Implement trong FileVault

### Header

```cpp
// include/filevault/algorithms/classical/caesar.hpp

class Caesar : public core::CryptoAlgorithm {
public:
    explicit Caesar(int shift = 3);  // Default: Caesar's own choice
    
    std::string name() const override { return "Caesar"; }
    core::AlgorithmType type() const override { 
        return core::AlgorithmType::CAESAR; 
    }
    
    core::CryptoResult encrypt(
        std::span<const uint8_t> plaintext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    core::CryptoResult decrypt(
        std::span<const uint8_t> ciphertext,
        std::span<const uint8_t> key,
        const core::EncryptionConfig& config) override;
    
    // Educational: Brute force all 26 possibilities
    std::string brute_force(const std::string& ciphertext);
    
private:
    int shift_;
    char shift_char(char ch, int shift) const;
};
```

### Implementation

```cpp
// src/algorithms/classical/caesar.cpp

Caesar::Caesar(int shift) : shift_(shift % 26) {}

char Caesar::shift_char(char ch, int shift) const {
    // Only shift alphabetic characters
    if (std::isalpha(ch)) {
        char base = std::isupper(ch) ? 'A' : 'a';
        // Normalize to 0-25, shift, wrap around, denormalize
        return base + (ch - base + shift + 26) % 26;
    }
    // Leave non-alphabetic characters unchanged
    return ch;
}

core::CryptoResult Caesar::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& /* config */) 
{
    // Allow key override (first byte = shift amount)
    int shift = shift_;
    if (!key.empty()) {
        shift = static_cast<int>(key[0]) % 26;
    }
    
    std::vector<uint8_t> result;
    result.reserve(plaintext.size());
    
    for (uint8_t byte : plaintext) {
        char ch = static_cast<char>(byte);
        char shifted = shift_char(ch, shift);
        result.push_back(static_cast<uint8_t>(shifted));
    }
    
    return core::CryptoResult{
        .success = true,
        .data = std::move(result),
        .algorithm_used = core::AlgorithmType::CAESAR
    };
}

core::CryptoResult Caesar::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config) 
{
    int shift = shift_;
    if (!key.empty()) {
        shift = static_cast<int>(key[0]) % 26;
    }
    
    // Decrypt = encrypt with negative shift
    std::vector<uint8_t> result;
    for (uint8_t byte : ciphertext) {
        result.push_back(
            static_cast<uint8_t>(shift_char(static_cast<char>(byte), -shift))
        );
    }
    
    return core::CryptoResult{
        .success = true,
        .data = std::move(result)
    };
}
```

---

## 🔓 Phương pháp tấn công

### 1. Brute Force Attack

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Brute Force Attack                               │
└─────────────────────────────────────────────────────────────────────┘

Key space = 26 possibilities (shift 0-25)
→ Try tất cả trong < 1 giây!

    Ciphertext: "KHOOR ZRUOG"
    
    Shift 0:  KHOOR ZRUOG  ← Gibberish
    Shift 1:  JGNNQ YQTNF  ← Gibberish
    Shift 2:  IFMMP XPSME  ← Gibberish
    Shift 3:  HELLO WORLD  ← ✓ READABLE!  ← Found it!
    Shift 4:  GDKKN VNQKC  ← Gibberish
    ...
    
    Complexity: O(26) = O(1) ← Trivial!
```

### 2. Frequency Analysis

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Frequency Analysis                               │
└─────────────────────────────────────────────────────────────────────┘

English letter frequency:
    
    E ████████████████████████████ 12.7%
    T ████████████████████████ 9.1%
    A ███████████████████████ 8.2%
    O ██████████████████████ 7.5%
    I █████████████████████ 7.0%
    N █████████████████████ 6.7%
    ...
    Z █ 0.07%

Attack:
1. Count letter frequency trong ciphertext
2. Chữ phổ biến nhất → likely = E encrypted
3. Tính shift từ most frequent letter
4. Verify và adjust

Example:
    Ciphertext frequency: H = 15%  (most common)
    E is usually most common
    Shift = H - E = 7 - 4 = 3 ✓
```

### FileVault Brute Force Implementation

```cpp
std::string Caesar::brute_force(const std::string& ciphertext) {
    std::ostringstream result;
    result << "Caesar Brute Force Attack:\n";
    result << "==========================\n\n";
    
    for (int shift = 0; shift < 26; ++shift) {
        result << "Shift " << std::setw(2) << shift << ": ";
        
        std::string decrypted;
        for (char ch : ciphertext) {
            decrypted += shift_char(ch, -shift);
        }
        
        result << decrypted << "\n";
    }
    
    return result.str();
}
```

---

## 🎮 Demo Brute Force

```bash
# Encrypt a message
$ filevault encrypt secret.txt -a caesar

# Now crack it!
$ filevault crack secret.txt.fvlt --brute-force

Caesar Brute Force Attack:
==========================

Shift  0: KHOOR ZRUOG
Shift  1: JGNNQ YQTNF
Shift  2: IFMMP XPSME
Shift  3: HELLO WORLD  ← ✓ THIS IS IT!
Shift  4: GDKKN VNQKC
Shift  5: FCJJM UMPJB
...
Shift 25: LIPPS ASVPH

[!] Possible plaintext found at shift 3
```

---

## 📝 Bài tập thực hành

### Bài 1: Giải mã thủ công

```
Ciphertext: "WKH TXLFN EURZQ IRA MXPSV RYHU WKH ODCB GRJ"
Shift: 3

Hint: Đây là câu pangram nổi tiếng

Solution steps:
1. W → ? (W - 3 = T)
2. K → ? (K - 3 = H)
3. ...

Answer: ________________________________
```

### Bài 2: Tìm shift từ ciphertext

```
Ciphertext: "LIPPS ASVPH"

Step 1: Đếm frequency
Step 2: Most common = ?
Step 3: Assume most common = E
Step 4: Calculate shift = ?

Answer: Shift = ___
        Plaintext = ________________________________
```

### Bài 3: Implement frequency analysis

```python
def frequency_analysis(ciphertext):
    """
    TODO: Count letter frequency và guess shift
    """
    freq = {}
    for char in ciphertext.upper():
        if char.isalpha():
            freq[char] = freq.get(char, 0) + 1
    
    # Find most common letter
    most_common = max(freq, key=freq.get)
    
    # Assume it's 'E' (most common in English)
    shift = (ord(most_common) - ord('E')) % 26
    
    return shift

# Test
ciphertext = "KHOOR ZRUOG"
shift = frequency_analysis(ciphertext)
print(f"Predicted shift: {shift}")
```

---

## 🔄 So sánh với Mật mã Hiện đại

```
┌───────────────────────────────────────────────────────────────────────┐
│                 Caesar vs Modern Encryption                           │
├───────────────┬────────────────────┬──────────────────────────────────┤
│   Property    │      Caesar        │         AES-256-GCM              │
├───────────────┼────────────────────┼──────────────────────────────────┤
│ Key space     │ 26                 │ 2^256                            │
│ Brute force   │ < 1 second         │ Heat death of universe           │
│ Frequency     │ Trivial            │ Impossible                       │
│ Key size      │ 5 bits             │ 256 bits                         │
│ Block size    │ 1 character        │ 128 bits                         │
│ Security      │ 0 bits             │ 256 bits                         │
│ Use case      │ Education only     │ Production                       │
└───────────────┴────────────────────┴──────────────────────────────────┘
```

---

## 🎓 Lessons Learned

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Key Takeaways                                    │
└─────────────────────────────────────────────────────────────────────┘

1. KEY SPACE MATTERS
   Caesar: 26 keys → trivial brute force
   AES-256: 2^256 keys → impossible brute force

2. FREQUENCY ANALYSIS
   Any substitution cipher preserves letter frequency
   → Modern ciphers: block operations, diffusion

3. KERCKHOFFS'S PRINCIPLE  
   Security should depend on KEY, not algorithm secrecy
   Caesar fails: knowing algorithm = cracked

4. DEFENSE IN DEPTH
   One layer of weak crypto = no security
   Modern: multiple layers, AEAD, KDF, etc.
```

---

## 📚 Tham khảo

1. **"The Code Book"** - Simon Singh
   - Lịch sử mật mã học từ Caesar đến quantum

2. **Cryptography I** - Stanford (Coursera)
   - https://www.coursera.org/learn/crypto

3. **"Applied Cryptography"** - Bruce Schneier
   - Chapter 1: Classical ciphers

---

<div align="center">

**[← Classical README](./README.md)** | **[Vigenère →](./vigenere.md)**

</div>
