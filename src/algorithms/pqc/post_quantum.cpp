/**
 * @file post_quantum.cpp
 * @brief Post-Quantum Cryptography implementations using Botan 3.x
 * 
 * Implements Kyber (ML-KEM) and Dilithium (ML-DSA) from NIST PQC standards.
 */

#include "filevault/algorithms/pqc/post_quantum.hpp"
#include "filevault/algorithms/symmetric/aes_gcm.hpp"
#include <botan/auto_rng.h>
#include <botan/pubkey.h>
#include <botan/pk_keys.h>
#include <botan/kyber.h>
#include <botan/dilithium.h>
#include <botan/secmem.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace filevault {
namespace algorithms {
namespace pqc {

// Helper to get KyberMode from Variant
static Botan::KyberMode get_kyber_mode(Kyber::Variant variant) {
    switch (variant) {
        case Kyber::Variant::Kyber512:
            return Botan::KyberMode::Kyber512_R3;
        case Kyber::Variant::Kyber768:
            return Botan::KyberMode::Kyber768_R3;
        case Kyber::Variant::Kyber1024:
            return Botan::KyberMode::Kyber1024_R3;
        default:
            return Botan::KyberMode::Kyber768_R3;
    }
}

// Helper to get DilithiumMode from Variant
static Botan::DilithiumMode get_dilithium_mode(Dilithium::Variant variant) {
    switch (variant) {
        case Dilithium::Variant::Dilithium2:
            return Botan::DilithiumMode::Dilithium4x4;
        case Dilithium::Variant::Dilithium3:
            return Botan::DilithiumMode::Dilithium6x5;
        case Dilithium::Variant::Dilithium5:
            return Botan::DilithiumMode::Dilithium8x7;
        default:
            return Botan::DilithiumMode::Dilithium6x5;
    }
}

// ============================================================================
// Kyber Implementation
// ============================================================================

Kyber::Kyber(Variant variant) : variant_(variant) {
    switch (variant) {
        case Variant::Kyber512:
            type_ = core::AlgorithmType::KYBER_512;
            break;
        case Variant::Kyber768:
            type_ = core::AlgorithmType::KYBER_768;
            break;
        case Variant::Kyber1024:
            type_ = core::AlgorithmType::KYBER_1024;
            break;
    }
    spdlog::debug("Created Kyber-{} algorithm", 
        variant == Variant::Kyber512 ? "512" : 
        variant == Variant::Kyber768 ? "768" : "1024");
}

std::string Kyber::name() const {
    switch (variant_) {
        case Variant::Kyber512: return "Kyber-512";
        case Variant::Kyber768: return "Kyber-768";
        case Variant::Kyber1024: return "Kyber-1024";
    }
    return "Kyber";
}

core::AlgorithmType Kyber::type() const {
    return type_;
}

size_t Kyber::key_size() const {
    return 32;  // Shared secret is always 32 bytes
}

size_t Kyber::public_key_size() const {
    switch (variant_) {
        case Variant::Kyber512: return 800;
        case Variant::Kyber768: return 1184;
        case Variant::Kyber1024: return 1568;
    }
    return 0;
}

size_t Kyber::private_key_size() const {
    switch (variant_) {
        case Variant::Kyber512: return 1632;
        case Variant::Kyber768: return 2400;
        case Variant::Kyber1024: return 3168;
    }
    return 0;
}

size_t Kyber::ciphertext_size() const {
    switch (variant_) {
        case Variant::Kyber512: return 768;
        case Variant::Kyber768: return 1088;
        case Variant::Kyber1024: return 1568;
    }
    return 0;
}

bool Kyber::is_suitable_for(core::SecurityLevel level) const {
    switch (level) {
        case core::SecurityLevel::WEAK:
        case core::SecurityLevel::MEDIUM:
            return variant_ >= Variant::Kyber512;
        case core::SecurityLevel::STRONG:
            return variant_ >= Variant::Kyber768;
        case core::SecurityLevel::PARANOID:
            return variant_ == Variant::Kyber1024;
        default:
            return false;
    }
}

PQKeyPair Kyber::generate_keypair() {
    PQKeyPair result;
    result.algorithm = name();
    
    try {
        Botan::AutoSeeded_RNG rng;
        Botan::KyberMode mode = get_kyber_mode(variant_);
        
        Botan::Kyber_PrivateKey private_key(rng, mode);
        
        auto priv_bits = private_key.raw_private_key_bits();
        auto pub_bits = private_key.public_key_bits();
        result.private_key.assign(priv_bits.begin(), priv_bits.end());
        result.public_key.assign(pub_bits.begin(), pub_bits.end());
        
        spdlog::info("Generated {} key pair: public={} bytes, private={} bytes",
                     name(), result.public_key.size(), result.private_key.size());
        
    } catch (const std::exception& e) {
        spdlog::error("Kyber key generation failed: {}", e.what());
        throw;
    }
    
    return result;
}

core::CryptoResult Kyber::encrypt(
    [[maybe_unused]] std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    [[maybe_unused]] const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        Botan::AutoSeeded_RNG rng;
        Botan::KyberMode mode = get_kyber_mode(variant_);
        
        // Load public key
        std::vector<uint8_t> pub_key_vec(key.begin(), key.end());
        Botan::Kyber_PublicKey public_key(pub_key_vec, mode);
        
        // Create encapsulator
        Botan::PK_KEM_Encryptor encryptor(public_key, "Raw");
        
        // Encapsulate: returns KEM_Encapsulation with encapsulated_shared_key and shared_key
        auto kem_result = encryptor.encrypt(rng, 32);
        
        // Store encapsulated key (ciphertext)
        const auto& encap_key = kem_result.encapsulated_shared_key();
        result.data.assign(encap_key.begin(), encap_key.end());
        
        // Store shared secret in nonce field (repurposed for KEM)
        const auto& shared = kem_result.shared_key();
        result.nonce = std::vector<uint8_t>(shared.begin(), shared.end());
        
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = 0;
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("Kyber encapsulation: ciphertext={} bytes, shared_secret=32 bytes",
                      result.data.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Kyber encapsulation failed: ") + e.what();
        spdlog::error("{}", result.error_message);
    }
    
    return result;
}

core::CryptoResult Kyber::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    [[maybe_unused]] const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        Botan::AutoSeeded_RNG rng;
        Botan::KyberMode mode = get_kyber_mode(variant_);
        
        // Load private key
        std::vector<uint8_t> priv_key_vec(key.begin(), key.end());
        Botan::Kyber_PrivateKey private_key(priv_key_vec, mode);
        
        // Create decapsulator
        Botan::PK_KEM_Decryptor decryptor(private_key, rng, "Raw");
        
        // Decapsulate to get shared secret
        auto shared_secret = decryptor.decrypt(ciphertext, 32);
        
        // Store shared secret
        result.data.assign(shared_secret.begin(), shared_secret.end());
        
        result.success = true;
        result.algorithm_used = type_;
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("Kyber decapsulation: shared_secret={} bytes", result.data.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Kyber decapsulation failed: ") + e.what();
        spdlog::error("{}", result.error_message);
    }
    
    return result;
}

// ============================================================================
// Dilithium Implementation
// ============================================================================

Dilithium::Dilithium(Variant variant) : variant_(variant) {
    switch (variant) {
        case Variant::Dilithium2:
            botan_name_ = "Dilithium-4x4-r3";
            break;
        case Variant::Dilithium3:
            botan_name_ = "Dilithium-6x5-r3";
            break;
        case Variant::Dilithium5:
            botan_name_ = "Dilithium-8x7-r3";
            break;
    }
    
    spdlog::debug("Created {} signature algorithm", botan_name_);
}

std::string Dilithium::name() const {
    switch (variant_) {
        case Variant::Dilithium2: return "Dilithium2";
        case Variant::Dilithium3: return "Dilithium3";
        case Variant::Dilithium5: return "Dilithium5";
    }
    return "Dilithium";
}

size_t Dilithium::public_key_size() const {
    switch (variant_) {
        case Variant::Dilithium2: return 1312;
        case Variant::Dilithium3: return 1952;
        case Variant::Dilithium5: return 2592;
    }
    return 0;
}

size_t Dilithium::private_key_size() const {
    switch (variant_) {
        case Variant::Dilithium2: return 2560;
        case Variant::Dilithium3: return 4032;
        case Variant::Dilithium5: return 4896;
    }
    return 0;
}

size_t Dilithium::signature_size() const {
    switch (variant_) {
        case Variant::Dilithium2: return 2420;
        case Variant::Dilithium3: return 3293;
        case Variant::Dilithium5: return 4595;
    }
    return 0;
}

PQKeyPair Dilithium::generate_keypair() {
    PQKeyPair result;
    result.algorithm = name();
    
    try {
        Botan::AutoSeeded_RNG rng;
        Botan::DilithiumMode mode = get_dilithium_mode(variant_);
        
        Botan::Dilithium_PrivateKey private_key(rng, mode);
        
        auto priv_bits = private_key.raw_private_key_bits();
        auto pub_bits = private_key.public_key_bits();
        result.private_key.assign(priv_bits.begin(), priv_bits.end());
        result.public_key.assign(pub_bits.begin(), pub_bits.end());
        
        spdlog::info("Generated {} key pair: public={} bytes, private={} bytes",
                     name(), result.public_key.size(), result.private_key.size());
        
    } catch (const std::exception& e) {
        spdlog::error("Dilithium key generation failed: {}", e.what());
        throw;
    }
    
    return result;
}

std::vector<uint8_t> Dilithium::sign(
    std::span<const uint8_t> message,
    std::span<const uint8_t> private_key
) {
    try {
        Botan::AutoSeeded_RNG rng;
        Botan::DilithiumMode mode = get_dilithium_mode(variant_);
        
        // Load private key
        std::vector<uint8_t> priv_key_vec(private_key.begin(), private_key.end());
        Botan::Dilithium_PrivateKey key(priv_key_vec, mode);
        
        // Create signer
        Botan::PK_Signer signer(key, rng, "");
        
        // Sign message
        auto signature = signer.sign_message(message.data(), message.size(), rng);
        
        spdlog::debug("Signed message: {} bytes -> signature {} bytes",
                      message.size(), signature.size());
        
        return signature;
        
    } catch (const std::exception& e) {
        spdlog::error("Dilithium signing failed: {}", e.what());
        throw;
    }
}

bool Dilithium::verify(
    std::span<const uint8_t> message,
    std::span<const uint8_t> signature,
    std::span<const uint8_t> public_key
) {
    try {
        Botan::DilithiumMode mode = get_dilithium_mode(variant_);
        
        // Load public key
        std::vector<uint8_t> pub_key_vec(public_key.begin(), public_key.end());
        Botan::Dilithium_PublicKey key(pub_key_vec, mode);
        
        // Create verifier
        Botan::PK_Verifier verifier(key, "");
        
        // Verify signature
        bool valid = verifier.verify_message(
            message.data(), message.size(),
            signature.data(), signature.size()
        );
        
        spdlog::debug("Signature verification: {}", valid ? "VALID" : "INVALID");
        
        return valid;
        
    } catch (const std::exception& e) {
        spdlog::error("Dilithium verification failed: {}", e.what());
        return false;
    }
}

// ============================================================================
// KyberHybrid Implementation (Kyber + AES-256-GCM)
// ============================================================================

KyberHybrid::KyberHybrid(Kyber::Variant variant) : kyber_(variant) {
    spdlog::debug("Created KyberHybrid with {}", kyber_.name());
}

std::string KyberHybrid::name() const {
    return kyber_.name() + "-Hybrid";
}

core::AlgorithmType KyberHybrid::type() const {
    switch (kyber_.type()) {
        case core::AlgorithmType::KYBER_512:
            return core::AlgorithmType::KYBER_512_HYBRID;
        case core::AlgorithmType::KYBER_768:
            return core::AlgorithmType::KYBER_768_HYBRID;
        case core::AlgorithmType::KYBER_1024:
            return core::AlgorithmType::KYBER_1024_HYBRID;
        default:
            return core::AlgorithmType::KYBER_768_HYBRID;
    }
}

size_t KyberHybrid::key_size() const {
    return 32; // AES-256 key size
}

bool KyberHybrid::is_suitable_for(core::SecurityLevel level) const {
    return kyber_.is_suitable_for(level);
}

PQKeyPair KyberHybrid::generate_keypair() {
    return kyber_.generate_keypair();
}

core::CryptoResult KyberHybrid::encrypt(
    std::span<const uint8_t> plaintext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        // Step 1: Encapsulate to get shared secret
        core::EncryptionConfig kem_config = config;
        auto kem_result = kyber_.encrypt({}, key, kem_config);
        
        if (!kem_result.success) {
            result.success = false;
            result.error_message = "KEM encapsulation failed: " + kem_result.error_message;
            return result;
        }
        
        // kem_result.data contains encapsulated key
        // kem_result.nonce contains shared secret (32 bytes)
        
        // Step 2: Use shared secret as AES-256-GCM key
        symmetric::AES_GCM aes(256);
        
        core::EncryptionConfig aes_config = config;
        aes_config.algorithm = core::AlgorithmType::AES_256_GCM;
        
        // Extract shared secret from optional
        if (!kem_result.nonce.has_value()) {
            result.success = false;
            result.error_message = "KEM encapsulation did not produce shared secret";
            return result;
        }
        const auto& shared_secret = *kem_result.nonce;
        auto aes_result = aes.encrypt(plaintext, shared_secret, aes_config);
        
        if (!aes_result.success) {
            result.success = false;
            result.error_message = "AES-GCM encryption failed: " + aes_result.error_message;
            return result;
        }
        
        // Extract AES nonce
        if (!aes_result.nonce.has_value()) {
            result.success = false;
            result.error_message = "AES-GCM encryption did not produce nonce";
            return result;
        }
        const auto& aes_nonce = *aes_result.nonce;
        
        // Extract AES tag
        if (!aes_result.tag.has_value()) {
            result.success = false;
            result.error_message = "AES-GCM encryption did not produce tag";
            return result;
        }
        const auto& aes_tag = *aes_result.tag;
        
        // Step 3: Combine: [kem_ct_len (4)][kem_ct][aes_nonce (12)][aes_tag (16)][aes_ciphertext]
        uint32_t kem_ct_len = static_cast<uint32_t>(kem_result.data.size());
        
        result.data.reserve(4 + kem_result.data.size() + aes_nonce.size() + aes_tag.size() + aes_result.data.size());
        
        // Write KEM ciphertext length (4 bytes, little-endian)
        result.data.push_back(static_cast<uint8_t>(kem_ct_len & 0xFF));
        result.data.push_back(static_cast<uint8_t>((kem_ct_len >> 8) & 0xFF));
        result.data.push_back(static_cast<uint8_t>((kem_ct_len >> 16) & 0xFF));
        result.data.push_back(static_cast<uint8_t>((kem_ct_len >> 24) & 0xFF));
        
        // Append KEM ciphertext
        result.data.insert(result.data.end(), kem_result.data.begin(), kem_result.data.end());
        
        // Append AES nonce (12 bytes)
        result.data.insert(result.data.end(), aes_nonce.begin(), aes_nonce.end());
        
        // Append AES tag (16 bytes)
        result.data.insert(result.data.end(), aes_tag.begin(), aes_tag.end());
        
        // Append AES ciphertext
        result.data.insert(result.data.end(), aes_result.data.begin(), aes_result.data.end());
        
        result.success = true;
        result.algorithm_used = type();
        result.original_size = plaintext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("KyberHybrid encrypt: {} bytes -> {} bytes",
                      plaintext.size(), result.data.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("KyberHybrid encryption failed: ") + e.what();
        spdlog::error("{}", result.error_message);
    }
    
    return result;
}

core::CryptoResult KyberHybrid::decrypt(
    std::span<const uint8_t> ciphertext,
    std::span<const uint8_t> key,
    const core::EncryptionConfig& config
) {
    auto start = std::chrono::high_resolution_clock::now();
    core::CryptoResult result;
    
    try {
        if (ciphertext.size() < 4) {
            result.success = false;
            result.error_message = "Ciphertext too short";
            return result;
        }
        
        // Step 1: Parse header
        size_t offset = 0;
        
        // Read KEM ciphertext length (4 bytes, little-endian)
        uint32_t kem_ct_len = 
            static_cast<uint32_t>(ciphertext[offset]) |
            (static_cast<uint32_t>(ciphertext[offset + 1]) << 8) |
            (static_cast<uint32_t>(ciphertext[offset + 2]) << 16) |
            (static_cast<uint32_t>(ciphertext[offset + 3]) << 24);
        offset += 4;
        
        if (offset + kem_ct_len + 12 + 16 > ciphertext.size()) {
            result.success = false;
            result.error_message = "Invalid ciphertext format";
            return result;
        }
        
        // Extract KEM ciphertext
        std::span<const uint8_t> kem_ct = ciphertext.subspan(offset, kem_ct_len);
        offset += kem_ct_len;
        
        // Extract AES nonce (12 bytes for GCM)
        std::span<const uint8_t> aes_nonce = ciphertext.subspan(offset, 12);
        offset += 12;
        
        // Extract AES tag (16 bytes for GCM)
        std::span<const uint8_t> aes_tag = ciphertext.subspan(offset, 16);
        offset += 16;
        
        // Rest is AES ciphertext
        std::span<const uint8_t> aes_ct = ciphertext.subspan(offset);
        
        // Step 2: Decapsulate to get shared secret
        core::EncryptionConfig kem_config = config;
        auto kem_result = kyber_.decrypt(kem_ct, key, kem_config);
        
        if (!kem_result.success) {
            result.success = false;
            result.error_message = "KEM decapsulation failed: " + kem_result.error_message;
            return result;
        }
        
        // kem_result.data contains the shared secret (32 bytes)
        
        // Step 3: Decrypt with AES-256-GCM using shared secret as key
        symmetric::AES_GCM aes(256);
        
        core::EncryptionConfig aes_config = config;
        aes_config.algorithm = core::AlgorithmType::AES_256_GCM;
        aes_config.nonce = std::vector<uint8_t>(aes_nonce.begin(), aes_nonce.end());
        aes_config.tag = std::vector<uint8_t>(aes_tag.begin(), aes_tag.end());
        
        auto aes_result = aes.decrypt(aes_ct, kem_result.data, aes_config);
        
        if (!aes_result.success) {
            result.success = false;
            result.error_message = "AES-GCM decryption failed: " + aes_result.error_message;
            return result;
        }
        
        result.data = std::move(aes_result.data);
        result.success = true;
        result.algorithm_used = type();
        result.original_size = ciphertext.size();
        result.final_size = result.data.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        spdlog::debug("KyberHybrid decrypt: {} bytes -> {} bytes",
                      ciphertext.size(), result.data.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("KyberHybrid decryption failed: ") + e.what();
        spdlog::error("{}", result.error_message);
    }
    
    return result;
}

} // namespace pqc
} // namespace algorithms
} // namespace filevault
