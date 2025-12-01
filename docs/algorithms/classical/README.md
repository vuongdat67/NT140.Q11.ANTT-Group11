# 📚 Classical Ciphers - Mật mã Cổ điển

> **CHỈ DÀNH CHO HỌC TẬP VÀ GIÁO DỤC - KHÔNG AN TOÀN CHO SỬ DỤNG THỰC TẾ!**

---

## ⚠️ Cảnh báo Quan trọng

```
╔══════════════════════════════════════════════════════════════════════╗
║                    ⚠️  SECURITY WARNING  ⚠️                          ║
╠══════════════════════════════════════════════════════════════════════╣
║                                                                       ║
║  TẤT CẢ các thuật toán trong folder này ĐÃ BỊ PHÁ!                   ║
║                                                                       ║
║  ├── Caesar: Brute force trong < 1 giây                              ║
║  ├── Vigenère: Kasiski examination + frequency analysis             ║
║  ├── Playfair: Frequency analysis on digraphs                       ║
║  ├── Hill: Known-plaintext attack (linear algebra)                  ║
║  └── Substitution: Frequency analysis                               ║
║                                                                       ║
║  MỤC ĐÍCH: Học tập lịch sử mật mã, hiểu tại sao chúng thất bại      ║
║                                                                       ║
╚══════════════════════════════════════════════════════════════════════╝
```

---

## 📋 Danh sách thuật toán

| File | Algorithm | Type | Year | Security |
|------|-----------|------|------|----------|
| [caesar.md](./caesar.md) | Caesar | Shift | ~100 BC | 0 bits |
| [vigenere.md](./vigenere.md) | Vigenère | Polyalphabetic | 1553 | 0 bits |
| [playfair.md](./playfair.md) | Playfair | Digraph | 1854 | 0 bits |
| [hill.md](./hill.md) | Hill | Matrix | 1929 | 0 bits |
| [substitution.md](./substitution.md) | Substitution | Monoalphabetic | Ancient | 0 bits |

---

## 📜 Lịch sử Mật mã học

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Timeline of Cryptography                         │
└─────────────────────────────────────────────────────────────────────┘

    ~100 BC          1553           1854         1929          1977
       │               │              │            │             │
       ▼               ▼              ▼            ▼             ▼
    ┌──────┐       ┌────────┐    ┌────────┐   ┌──────┐      ┌─────┐
    │Caesar│       │Vigenère│    │Playfair│   │ Hill │      │ DES │
    │      │       │        │    │        │   │      │      │     │
    │Shift │       │Poly-   │    │Digraph │   │Matrix│      │Block│
    │cipher│       │alpha   │    │        │   │      │      │     │
    └──────┘       └────────┘    └────────┘   └──────┘      └─────┘
       │               │              │            │             │
       │          "Le chiffre        Used in     Linear       Modern
    Julius       indéchiffrable"     WWI        algebra       crypto
    Caesar        (unbreakable)                  based         begins
    
       │               │              │            │             │
       ▼               ▼              ▼            ▼             ▼
    Broken          Broken          Broken       Broken       Still
    instantly       1863            1900s        1930s        (sort of)
```

---

## 🔓 Phương pháp Tấn công

### 1. Brute Force

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Brute Force Attack                               │
└─────────────────────────────────────────────────────────────────────┘

    Algorithm      Key Space           Time to Break
    ─────────      ─────────           ─────────────
    Caesar         26                  < 1 second
    Vigenère       26^n (n=key len)    Minutes (with Kasiski)
    Substitution   26! ≈ 4×10²⁶       Still fast with frequency
    
    So sánh với modern:
    AES-256        2^256               Heat death of universe
```

### 2. Frequency Analysis

```
┌─────────────────────────────────────────────────────────────────────┐
│                    English Letter Frequency                         │
└─────────────────────────────────────────────────────────────────────┘

    E ████████████████████████████ 12.7%
    T ████████████████████████ 9.1%
    A ███████████████████████ 8.2%
    O ██████████████████████ 7.5%
    I █████████████████████ 7.0%
    N █████████████████████ 6.7%
    S ████████████████████ 6.3%
    H ████████████████████ 6.1%
    R ████████████████████ 6.0%
    ...
    Z █ 0.07%

Substitution cipher PRESERVES frequencies!
├── Most common in ciphertext → likely E
├── Common pairs (TH, HE, AN) → pattern matching
└── Given enough text → full recovery
```

### 3. Known-Plaintext Attack

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Known-Plaintext Attack                           │
└─────────────────────────────────────────────────────────────────────┘

If attacker knows some plaintext-ciphertext pairs:

Hill Cipher (3×3 matrix key K):
    C = K × P  (mod 26)
    
If we have 3 pairs (P₁,C₁), (P₂,C₂), (P₃,C₃):
    [C₁ C₂ C₃] = K × [P₁ P₂ P₃]
    K = [C₁ C₂ C₃] × [P₁ P₂ P₃]⁻¹ (mod 26)
    
    → Key recovered with linear algebra!
```

---

## 🎓 Bài học từ Mật mã Cổ điển

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Lessons Learned                                  │
└─────────────────────────────────────────────────────────────────────┘

1. KERCKHOFFS'S PRINCIPLE (1883)
   ────────────────────────────
   "Security should depend ONLY on the key, not the algorithm"
   
   Classical ciphers: Secrecy of algorithm required
   Modern ciphers: Algorithm public, key secret

2. KEY SPACE MATTERS
   ─────────────────
   Caesar: 26 keys → trivial
   AES-256: 2²⁵⁶ keys → impossible

3. DIFFUSION & CONFUSION (Claude Shannon, 1949)
   ─────────────────────────────────────────────
   Confusion: Each ciphertext bit depends on many key bits
   Diffusion: Each plaintext bit affects many ciphertext bits
   
   Classical: Poor diffusion (single letter → single letter)
   Modern: Excellent diffusion (1 bit change → ~50% output changes)

4. STATISTICAL PATTERNS
   ────────────────────
   Classical: Preserve language statistics
   Modern: Output indistinguishable from random

5. AUTHENTICATION
   ──────────────
   Classical: None (can modify ciphertext)
   Modern: AEAD (encryption + authentication together)
```

---

## 💻 Sử dụng trong FileVault

```bash
# ⚠️ CHỈ DÀNH CHO DEMO/HỌC TẬP!

# Caesar với shift 3
filevault encrypt message.txt -a caesar

# Vigenère với keyword
filevault encrypt message.txt -a vigenere -k "SECRET"

# Playfair
filevault encrypt message.txt -a playfair -k "KEYWORD"

# CẢNH BÁO sẽ hiện:
# [WARNING] Classical cipher detected!
# [WARNING] This provides NO security - for educational use only!
```

---

## 📖 Nội dung học tập

### Cơ bản → Nâng cao

```
1. [Caesar](./caesar.md)
   └── Đơn giản nhất, học về modular arithmetic
   
2. [Substitution](./substitution.md)
   └── Frequency analysis, ngôn ngữ học

3. [Vigenère](./vigenere.md)
   └── Polyalphabetic, Kasiski examination

4. [Playfair](./playfair.md)
   └── Digraph substitution, matrix operations

5. [Hill](./hill.md)
   └── Linear algebra, known-plaintext attacks
```

---

## 🔗 So sánh với Modern Crypto

```
┌────────────────┬────────────────────┬────────────────────────────────┐
│    Aspect      │     Classical      │           Modern               │
├────────────────┼────────────────────┼────────────────────────────────┤
│ Key space      │ Small (≤ 26!)      │ Huge (2¹²⁸ - 2²⁵⁶)            │
│ Operations     │ Character-based    │ Bit/block-based                │
│ Diffusion      │ Poor               │ Excellent (avalanche)          │
│ Authentication │ None               │ Built-in (AEAD)                │
│ Known attacks  │ Many               │ None practical                 │
│ Use today      │ Education only     │ Production                     │
└────────────────┴────────────────────┴────────────────────────────────┘
```

---

## 📚 Tài liệu tham khảo

1. **"The Code Book"** - Simon Singh
   - Lịch sử mật mã từ Caesar đến quantum
   
2. **"Applied Cryptography"** - Bruce Schneier
   - Chapter 1: Historical ciphers

3. **"Cryptography and Network Security"** - William Stallings
   - Classical cipher theory

4. **Coursera: Cryptography I** - Stanford
   - Week 1: History of cryptography

---

<div align="center">

**[← Quay lại Algorithms](../README.md)**

</div>
