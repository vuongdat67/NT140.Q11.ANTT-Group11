# Botan 3.x Reference Guide for FileVault

## 📚 Official Documentation

- **Main Documentation**: https://botan.randombit.net/handbook/
- **API Reference**: https://botan.randombit.net/handbook/api_ref/index.html
- **GitHub Repository**: https://github.com/randombit/botan
- **Migration Guide 2.x → 3.x**: https://botan.randombit.net/handbook/migration_3.html

---

## 🔧 Installation & Setup

### Ubuntu/Debian
```bash
# Botan 3.x from source
git clone https://github.com/randombit/botan.git
cd botan
./configure.py --prefix=/usr/local --cc=gcc --with-bzip2 --with-lzma --with-zlib
make -j$(nproc)
sudo make install
```

### macOS
```bash
brew install botan
```

### Windows (vcpkg)
```powershell
vcpkg install botan:x64-windows
```

### CMake Integration
```cmake
find_package(botan REQUIRED)
target_link_libraries(your_target PRIVATE botan::botan-3)
```

---

## 🎯 Botan 3.x Key Changes from 2.x

### 1. C++20 Requirement
```cpp
// Botan 3.x requires C++20
#include <botan/version.h>
static_assert(BOTAN_VERSION_MAJOR >= 3, "Botan 3.x required");
static_assert(__cplusplus >= 202002L, "C++20 required");
```

### 2. Namespace Changes
```cpp
// Botan 2.x
Botan::SymmetricKey key(key_data);

// Botan 3.x - More explicit types
Botan::secure_vector<uint8_t> key(key_data.begin(), key_data.end());
```

### 3. Updated API Patterns
```cpp
// Botan 2.x
auto cipher = get_cipher_mode("AES-256/GCM", ENCRYPTION);

// Botan 3.x - Factory pattern with smart pointers
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", 
                                         Botan::Cipher_Dir::Encryption);
```

---

## 🔐 Common Operations

### 1. Random Number Generation

```cpp
#include <botan/auto_rng.h>

// Create RNG
Botan::AutoSeeded_RNG rng;

// Generate random bytes
std::vector<uint8_t> random_data(32);
rng.randomize(random_data.data(), random_data.size());

// Generate random number in range
uint32_t random_num = rng.next_byte();  // 0-255
```

### 2. Hashing

```cpp
#include <botan/hash.h>

// Create hash function
auto hash = Botan::HashFunction::create("SHA-256");
if (!hash) {
    throw std::runtime_error("SHA-256 not available");
}

// Hash data
hash->update(data.data(), data.size());
auto result = hash->final();

// One-shot hashing
auto result = Botan::HashFunction::create_or_throw("SHA-256")
    ->process(data.data(), data.size());

// Available algorithms:
// - SHA-1, SHA-224, SHA-256, SHA-384, SHA-512
// - SHA-3-256, SHA-3-512
// - BLAKE2b, BLAKE2s
// - MD5 (legacy only)
```

### 3. Symmetric Encryption (AES-GCM)

```cpp
#include <botan/cipher_mode.h>
#include <botan/auto_rng.h>

// Create cipher
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", 
                                         Botan::Cipher_Dir::Encryption);
if (!cipher) {
    throw std::runtime_error("AES-256/GCM not available");
}

// Setup key
std::vector<uint8_t> key(32);  // 256 bits
rng.randomize(key.data(), key.size());
cipher->set_key(key);

// Generate nonce
std::vector<uint8_t> nonce(12);  // 96 bits for GCM
rng.randomize(nonce.data(), nonce.size());

// Encrypt
Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
cipher->start(nonce);
cipher->finish(buffer);
// buffer now contains: ciphertext + authentication tag (16 bytes)

// Decrypt
auto decipher = Botan::Cipher_Mode::create("AES-256/GCM",
                                           Botan::Cipher_Dir::Decryption);
decipher->set_key(key);
decipher->start(nonce);
decipher->finish(buffer);  // Verifies tag automatically
// buffer now contains: plaintext
```

**Available Cipher Modes:**
```cpp
// AEAD (Authenticated Encryption with Associated Data) - RECOMMENDED
"AES-128/GCM"
"AES-192/GCM" 
"AES-256/GCM"
"ChaCha20Poly1305"
"AES-128/OCB"
"AES-256/SIV"

// Traditional modes (need separate MAC) - NOT RECOMMENDED
"AES-256/CBC"
"AES-256/CTR"
"AES-256/CFB"
```

### 4. Key Derivation (Argon2)

```cpp
#include <botan/argon2.h>

// Argon2id (recommended - hybrid mode)
auto key = Botan::argon2_generate(
    32,                          // output_length (bytes)
    password.data(),             // password
    password.size(),             // password_length
    salt.data(),                 // salt
    salt.size(),                 // salt_length
    65536,                       // memory (KB)
    4,                           // parallelism
    100000                       // iterations
);

// For Argon2i or Argon2d, use:
#include <botan/pwdhash.h>

auto pwdhash = Botan::PasswordHashFamily::create("Argon2id");
std::vector<uint8_t> key(32);
pwdhash->derive_key(
    key.data(), key.size(),
    password.data(), password.size(),
    salt.data(), salt.size(),
    65536,     // memory
    100000,    // iterations  
    4          // parallelism
);
```

**Recommended Parameters (OWASP 2023):**
```cpp
constexpr struct {
    uint32_t memory_kb;
    uint32_t iterations;
    uint32_t parallelism;
} ARGON2_PARAMS = {
    .memory_kb = 65536,      // 64 MB
    .iterations = 3,         // 3 passes
    .parallelism = 4         // 4 threads
};

// Or use time-based calibration
auto params = Botan::Argon2::tune_parameters(
    100,    // target_ms (100ms delay acceptable)
    32      // output_length
);
```

### 5. Key Derivation (PBKDF2)

```cpp
#include <botan/pbkdf2.h>
#include <botan/hmac.h>

// Create PBKDF2 with HMAC-SHA256
auto mac = Botan::MessageAuthenticationCode::create("HMAC(SHA-256)");
auto pbkdf = std::make_unique<Botan::PBKDF2>(std::move(mac));

// Derive key
std::vector<uint8_t> key(32);
pbkdf->derive_key(
    key.data(), key.size(),
    password.data(), password.size(),
    salt.data(), salt.size(),
    310000  // OWASP 2023: 310,000 iterations
);
```

### 6. Message Authentication (HMAC)

```cpp
#include <botan/mac.h>

// Create HMAC
auto hmac = Botan::MessageAuthenticationCode::create("HMAC(SHA-256)");
hmac->set_key(key);

// Compute MAC
hmac->update(data);
auto tag = hmac->final();

// Verify MAC (constant-time)
hmac->update(data);
auto computed_tag = hmac->final();
bool valid = Botan::constant_time_compare(
    tag.data(), 
    computed_tag.data(), 
    tag.size()
);
```

### 7. RSA (Asymmetric)

```cpp
#include <botan/rsa.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>

// Generate RSA keypair
Botan::AutoSeeded_RNG rng;
Botan::RSA_PrivateKey private_key(rng, 3072);  // 3072-bit key

// Get public key
Botan::RSA_PublicKey public_key = private_key;

// Encrypt
Botan::PK_Encryptor_EME encryptor(public_key, rng, "OAEP(SHA-256)");
auto ciphertext = encryptor.encrypt(plaintext, rng);

// Decrypt
Botan::PK_Decryptor_EME decryptor(private_key, rng, "OAEP(SHA-256)");
auto decrypted = decryptor.decrypt(ciphertext);
```

### 8. Post-Quantum Cryptography (Kyber)

```cpp
#include <botan/kyber.h>

// Generate Kyber keypair (NIST Level 3 security)
Botan::AutoSeeded_RNG rng;
Botan::Kyber_PrivateKey private_key(rng, Botan::KyberMode::Kyber768_R3);
auto public_key = private_key.public_key();

// Encapsulate (create shared secret)
Botan::PK_KEM_Encryptor encapsulator(*public_key, "Raw");
auto kem_result = encapsulator.encrypt(rng, 32);  // 32-byte shared secret

const auto& ciphertext = kem_result.encapsulated_shared_key();
const auto& shared_secret = kem_result.shared_key();

// Decapsulate (recover shared secret)
Botan::PK_KEM_Decryptor decapsulator(private_key, rng, "Raw");
auto recovered_secret = decapsulator.decrypt(ciphertext.data(), 
                                             ciphertext.size(), 
                                             32);

// Use shared_secret as AES key
```

### 9. Digital Signatures (Dilithium)

```cpp
#include <botan/dilithium.h>

// Generate Dilithium keypair
Botan::AutoSeeded_RNG rng;
Botan::Dilithium_PrivateKey private_key(rng, Botan::DilithiumMode::Dilithium6x5);

// Sign
Botan::PK_Signer signer(private_key, rng, "");
signer.update(message);
auto signature = signer.signature(rng);

// Verify
auto public_key = private_key.public_key();
Botan::PK_Verifier verifier(*public_key, "");
verifier.update(message);
bool valid = verifier.check_signature(signature);
```

---

## 🔧 Advanced Features

### 1. Compression Integration

```cpp
#include <botan/compression.h>

// Compress
auto compressor = Botan::Compression_Algorithm::create("zlib");
compressor->start(6);  // compression level 1-9
compressor->update(buffer);
compressor->finish(buffer);

// Decompress
auto decompressor = Botan::Decompression_Algorithm::create("zlib");
decompressor->start();
decompressor->update(buffer);
decompressor->finish(buffer);

// Available: "zlib", "bzip2", "lzma"
```

### 2. Base64 Encoding

```cpp
#include <botan/base64.h>

// Encode
std::string b64 = Botan::base64_encode(data.data(), data.size());

// Decode
auto decoded = Botan::base64_decode(b64);
```

### 3. Hex Encoding

```cpp
#include <botan/hex.h>

// Encode
std::string hex = Botan::hex_encode(data.data(), data.size());

// Decode
auto decoded = Botan::hex_decode(hex);
```

### 4. Secure Memory

```cpp
#include <botan/secmem.h>

// Secure vector (auto-zeroed on destruction)
Botan::secure_vector<uint8_t> key(32);

// Locked memory (won't be swapped to disk)
Botan::secure_vector<uint8_t> sensitive_data = 
    Botan::lock_mem(data.begin(), data.end());
```

### 5. Key Wrap (RFC 3394)

```cpp
#include <botan/rfc3394.h>

// Wrap key with KEK
auto wrapped = Botan::rfc3394_keywrap(
    key_to_wrap.data(),
    key_to_wrap.size(),
    key_encryption_key
);

// Unwrap
auto unwrapped = Botan::rfc3394_keyunwrap(
    wrapped.data(),
    wrapped.size(),
    key_encryption_key
);
```

### 6. Constant-Time Operations

```cpp
#include <botan/ct_utils.h>

// Constant-time comparison
bool equal = Botan::constant_time_compare(a.data(), b.data(), a.size());

// Constant-time conditional
auto selected = Botan::CT::Mask<uint8_t>::is_zero(condition).select(
    if_true_value,
    if_false_value
);
```

---

## 🎨 Error Handling

```cpp
#include <botan/exceptn.h>

try {
    auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", 
                                             Botan::Cipher_Dir::Encryption);
    if (!cipher) {
        throw std::runtime_error("Algorithm not available");
    }
    
    cipher->set_key(key);
    cipher->start(nonce);
    cipher->finish(buffer);
    
} catch (const Botan::Invalid_Key_Length& e) {
    // Wrong key size
    std::cerr << "Key error: " << e.what() << std::endl;
    
} catch (const Botan::Invalid_IV_Length& e) {
    // Wrong IV/nonce size
    std::cerr << "IV error: " << e.what() << std::endl;
    
} catch (const Botan::Integrity_Failure& e) {
    // Authentication tag verification failed
    std::cerr << "Authentication failed: " << e.what() << std::endl;
    
} catch (const Botan::Exception& e) {
    // General Botan exception
    std::cerr << "Botan error: " << e.what() << std::endl;
    
} catch (const std::exception& e) {
    // Other errors
    std::cerr << "Error: " << e.what() << std::endl;
}
```

---

## 📊 Performance Tips

### 1. Reuse Objects
```cpp
// ❌ BAD: Creating cipher every time
for (const auto& block : blocks) {
    auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", ...);
    cipher->process(block);  // Slow!
}

// ✅ GOOD: Reuse cipher
auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", ...);
for (const auto& block : blocks) {
    cipher->start(generate_nonce());
    cipher->process(block);  // Fast!
}
```

### 2. Use Hardware Acceleration
```cpp
// Botan automatically uses:
// - AES-NI for AES
// - SHA-NI for SHA
// - AVX2/AVX-512 for ChaCha20

// Check if available:
if (Botan::CPUID::has_aes_ni()) {
    // AES will be hardware-accelerated
}
```

### 3. Batch Operations
```cpp
// Process large buffers instead of many small ones
const size_t CHUNK_SIZE = 1024 * 1024;  // 1MB chunks
```

---

## 🔍 Debugging

### Enable Debug Output
```cpp
// Build Botan with debug:
// ./configure.py --with-debug-info

// Runtime logging (if built with debug)
#include <botan/internal/loadstor.h>
```

### Memory Debugging
```cpp
// Use Valgrind
valgrind --leak-check=full ./your_program

// Use AddressSanitizer
// Compile with: -fsanitize=address
```

---

## 📚 Algorithm Reference

### Symmetric Ciphers
| Algorithm | Key Size | Block Size | Mode | Security |
|-----------|----------|------------|------|----------|
| AES-128 | 128 bits | 128 bits | GCM/CBC/CTR | High |
| AES-192 | 192 bits | 128 bits | GCM/CBC/CTR | High |
| AES-256 | 256 bits | 128 bits | GCM/CBC/CTR | Very High |
| ChaCha20 | 256 bits | Stream | Poly1305 | Very High |
| Serpent | 256 bits | 128 bits | GCM/CBC | Very High |

### Hash Functions
| Algorithm | Output Size | Security Level |
|-----------|-------------|----------------|
| SHA-256 | 256 bits | High |
| SHA-512 | 512 bits | Very High |
| SHA3-256 | 256 bits | High |
| BLAKE2b | 512 bits | Very High |

### KDF (Key Derivation)
| Algorithm | Type | Resistance |
|-----------|------|------------|
| Argon2id | Memory-hard | GPU/ASIC resistant |
| Argon2i | Memory-hard | Side-channel resistant |
| PBKDF2 | Iterative | Moderate |
| scrypt | Memory-hard | GPU resistant |

---

## 🎯 Best Practices

1. **Always use AEAD modes** (GCM, ChaCha20-Poly1305)
2. **Generate unique nonces** for every encryption
3. **Use Argon2id** for password-based key derivation
4. **Clear sensitive data** after use
5. **Use secure_vector** for keys and passwords
6. **Validate all inputs** before processing
7. **Use constant-time operations** for security-critical comparisons
8. **Test with NIST vectors** to verify correctness

---

## 🔗 Useful Links

- **API Docs**: https://botan.randombit.net/handbook/api_ref/
- **GitHub**: https://github.com/randombit/botan
- **Mailing List**: botan-devel@randombit.net
- **Security Advisories**: https://botan.randombit.net/security.html

---

## ✅ Quick Check

```cpp
// Verify Botan 3.x installation
#include <botan/version.h>
#include <iostream>

int main() {
    std::cout << "Botan version: " 
              << Botan::version_string() << "\n";
    std::cout << "Major: " << BOTAN_VERSION_MAJOR << "\n";
    std::cout << "Minor: " << BOTAN_VERSION_MINOR << "\n";
    std::cout << "Patch: " << BOTAN_VERSION_PATCH << "\n";
    
    if (BOTAN_VERSION_MAJOR >= 3) {
        std::cout << "✓ Botan 3.x detected\n";
    } else {
        std::cout << "✗ Need Botan 3.x or later\n";
    }
    
    return 0;
}
```

Compile and run:
```bash
g++ -std=c++20 check_botan.cpp -lbotan-3 -o check_botan
./check_botan
```